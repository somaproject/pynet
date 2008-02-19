#include "pynetevent.h"

static void
PyNetEvent_dealloc(PyNetEvent* self)
{

    self->ob_type->tp_free((PyObject*)self);
}

void walkEventList(struct eventListItem_t * li)
{
  struct eventListItem_t * current = li; 
  //printf("walking list beginning at %d\n", li); 
  while (current != NULL) {
    printf("e.cmd = %d, e.src = %d\n", 
	   current->e.cmd, 
	   current->e.src); 

    current = current->elt; 
  }

}

static PyObject *
PyNetEvent_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyNetEvent *self;

    self = (PyNetEvent *)type->tp_alloc(type, 0);
    if (self != NULL) {

	self->rxSet = PySet_New(0); 
	if (self->rxSet == NULL)
	  {
	    Py_DECREF(self); 
	    return NULL; 
	  }
	

	char * addrstr; 
	int ok = PyArg_ParseTuple(args, "s", &addrstr); 

	if (!ok) 
	  return NULL; 

	self->ip = PyString_FromString(addrstr); 
	

	// now the shared network state
	self->pnss = malloc(sizeof(struct NetworkSharedThreadState_t)); 

	pthread_mutex_init(&(self->pnss->running_mutex), NULL); 	
	self->pnss->running = 0; 
	
	// and the shared network state element list
	self->pnss->pel = malloc(sizeof(struct EventList_t)); 
	self->pnss->pel->size = 0; 

	pthread_mutex_init(&(self->pnss->pel->mutex), NULL); 
	pthread_mutex_init(&(self->pnss->pel->size_mutex), NULL); 

	
	pthread_cond_init(&(self->pnss->pel->size_thold_cv), NULL); 
	
       
	self->pnss->pel->eltHead = NULL; 

	self->pNetworkThread = malloc(sizeof(pthread_t)); 
	self->pnss->rxValidLUT = (char * ) malloc(256*256); 
	bzero(self->pnss->rxValidLUT, 256*256); 


        self->number = 0;
    }

    return (PyObject *)self;
}

static int
PyNetEvent_init(PyNetEvent *self, PyObject *args, PyObject *kwds)
{
    return 0;
}

void pthread_runner(struct NetworkSharedThreadState_t * pnss)
{
  
  struct EventList_t * pel = pnss->pel; 
  
  int socket = setupRXSocket(); 
  
  
  char firstLoop = 1; 
  uint32_t rxseq, prevseq =0; 
  char wasrunning; 

  while( 1) { 
    pthread_mutex_lock(&(pnss->running_mutex)); 

    wasrunning = pnss->running ; 

    pthread_mutex_unlock(&(pnss->running_mutex)); 

    if (wasrunning == 0 ) 
      break; 
    
    // now use select as a time-out-able interface
    fd_set readfds; 
    FD_SET(socket, &readfds); 
    struct timeval timeout; 
    timeout.tv_sec = 0; 
    timeout.tv_usec = 100000; 
    int retval = select(socket+1, &readfds, NULL, NULL, &timeout); 

    if(FD_ISSET(socket, &readfds) && retval > 0) {
    
      struct EventList_t newEventList;
      newEventList.eltHead = NULL;
      
      rxseq  = getEvents(socket, pnss->rxValidLUT, &newEventList);

      if (firstLoop) {
	firstLoop = 0;
	prevseq = rxseq -1;
      } 
	
      // check for sequential RX
      if (prevseq + 1 == rxseq ) {
	// good
	
	
	pthread_mutex_lock(&(pel->size_mutex)); 	
	  
	pthread_mutex_lock(&(pel->mutex));
	// if pel is empty, then we append this to it
	if (pel->eltHead == NULL) {
	  pel->eltHead = newEventList.eltHead; 
	  pel->eltTail = newEventList.eltTail; 
	} else { 
	  pel->eltTail->elt = newEventList.eltHead; 
	  pel->eltTail = newEventList.eltTail; 
	}
	
	
	// check if we added anything; if we did, update size
	if (newEventList.size > 0) 
	  {	  
	    pel->size += newEventList.size; 
	    //printf("added %d to pel, pel.size = %d\n", newEventList.size, pel->size); 
	  }
	
	pthread_mutex_unlock(&(pel->mutex));
	
	if (pel->size > 0 ){
	  
	  pthread_cond_signal(&(pel->size_thold_cv)); 
	}		
	
	pthread_mutex_unlock(&(pel->size_mutex)); 	  
	
      } else {
	printf("Oops, we dropped one, \n");
      }
      prevseq = rxseq; 
    }
  }
  

}

static PyObject * 
PyNetEvent_startEventRX(PyNetEvent* self)
{
  PyObject *iterator = PyObject_GetIter(self->rxSet);

  PyObject *item;

  if (iterator == NULL) {
    /* propagate error */
  }

  // build the constant-time LUT
  bzero(self->pnss->rxValidLUT, 256*256); 
  
  while ((item = PyIter_Next(iterator)) != 0) {
    /* do something with item */
    // get tuple
    if (PyTuple_Check(item)) {
      //int ts = PyTuple_Size(item); 
      //assert(ts == 2); 
      
      PyObject* popcmd = PyTuple_GetItem(item, 0); 
      long cmd = PyInt_AsLong(popcmd); 
      
      PyObject* popsrc = PyTuple_GetItem(item, 1); 
      long src = PyInt_AsLong(popsrc); 
      
      self->pnss->rxValidLUT[cmd + src*256] = 1; 
      
    } else {
      // error
      Py_RETURN_NONE; 
    }
    /* release reference when done */
    Py_DECREF(item);
  }

  // now we fork a thread and begin queueing up events
  pthread_mutex_lock(&(self->pnss->running_mutex)); 

  pthread_create((void*)&(self->pNetworkThread), NULL, 
		 (void *)pthread_runner, self->pnss); 


  self->pnss->running = 1; 
  pthread_mutex_unlock(&(self->pnss->running_mutex)); 
  	


  Py_DECREF(iterator);
  
  Py_RETURN_NONE; 
}

PyObject * eventToPyTuple(struct event_t * evt)
{
  /* Convert from a event_t to a python tuple;
     
  simply returning a tuple is just so much easier than a custom
  class or a numpy recordarray. 
  */
  
  PyObject * outtuple =  PyTuple_New(2+EVENTLEN-1); 
  uint8_t cmd = (unsigned char)evt->cmd; 
  uint8_t src = (unsigned char)evt->src; 
  PyTuple_SetItem(outtuple, 0, PyInt_FromLong(cmd)); 
  PyTuple_SetItem(outtuple, 1, PyInt_FromLong(src)); 
  int i = 0; 

  for (i = 0; i < EVENTLEN-1; i++ ){
    PyTuple_SetItem(outtuple, i+2, PyInt_FromLong((unsigned short)evt->data[i])); 
  }
  
  return outtuple; 
  
}

static PyObject * 
PyNetEvent_stopEventRX(PyNetEvent* self)
{
  //printf("PyNetEvent_stopEventRX(PyNetEvent* self)\n"); 

  pthread_mutex_lock(&(self->pnss->running_mutex)); 
  self->pnss->running = 0; 
  pthread_mutex_unlock(&(self->pnss->running_mutex)); 
  pthread_join(*(self->pNetworkThread), 0); 

  // flush the buffer
  
	  
  pthread_mutex_lock(&(self->pnss->pel->mutex));

  struct EventList_t * pel = self->pnss->pel; 
  struct eventListItem_t * p = pel->eltHead; 
  while (p != NULL) {
    struct eventListItem_t * nextp = p->elt; 
    free(p); 
    p = nextp; 
  }
  pel->eltHead = NULL; 
  pel->eltTail = NULL; 
  pthread_mutex_unlock(&(self->pnss->pel->mutex)); 

  pthread_mutex_lock(&(pel->size_mutex)); 
  pel->size =0; 
  pthread_mutex_unlock(&(pel->size_mutex)); 
  
  

  Py_RETURN_NONE; 

}

static PyObject *
PyNetEvent_getEvents(PyNetEvent* self)
{

  PyObject * outlist = PyList_New(0); 
  
  struct EventList_t * pel = self->pnss->pel; 

  // check the size
  /* fprintf("pre-size is %d\n",  */
/* 	  pel->size);  */

  pthread_mutex_lock(&(pel->size_mutex)) ;
/*   printf("PyNetEvent_getEvents(PyNetEvent* self): size is %d\n",   */
/*  	  pel->size);   */
  if (pel->size == 0 ) {
    
    pthread_cond_wait(&(pel->size_thold_cv), 
		      &(pel->size_mutex)); 
  }

  pthread_mutex_lock(&(pel->mutex)); 
  //walkEventList(pel->eltHead); 

  //printf("pel->mutex locked\n"); 
  struct eventListItem_t * phead = pel->eltHead; 

  pel->eltHead = NULL; 
  pel->eltTail = NULL; 
  pel->size = 0 ; 

  int pos = 0; 

  while(phead != NULL ) {
    //printf("here!\n");
    // there's a real element there!
    struct eventListItem_t * curhead = phead; 
    
    PyObject * outtuple = eventToPyTuple(&(curhead->e)); 
    PyList_Append(outlist, outtuple); 
    pos += 1; 
    phead = curhead->elt; 
    //printf("cmd %d, id %d %d\n", curhead->e.cmd, curhead->e.src, curhead->elt); 
    free(curhead); 
  }


  pthread_mutex_unlock(&(pel->mutex)); 
  pthread_mutex_unlock(&(pel->size_mutex)); 
  //printf("here!\n");
  return outlist; 

  
}

static PyObject*
PyNetEvent_send(PyNetEvent* self, PyObject *args, PyObject *kwds)
{
  
  struct event_t e; 
  uint8_t addrs[10]; 
  printf("Tuple parsed\n"); 
  int ok = PyArg_ParseTuple(args, "(BBBBBBBBBB)(BBHHHHH)",
			    &addrs[1], &addrs[0],
			    &addrs[3], &addrs[2],  
			    &addrs[5], &addrs[4], 
			    &addrs[7], &addrs[6], 
			    &addrs[9], &addrs[8], 
			    &e.cmd, &e.src, 
			    &e.data[0], &e.data[1], &e.data[2], 
			    &e.data[3], &e.data[4]); 
  
  if (!ok) 
    return NULL; 

  struct sockaddr_in saServer; 
  
  
  int sock = socket(AF_INET, SOCK_DGRAM, 17); 
  memset(&saServer, sizeof(saServer), 0); 
  saServer.sin_family = AF_INET; 
  saServer.sin_port = htons(EVENTTXPORT);  
  char * ip = PyString_AsString(self->ip); 

  inet_aton(ip, &saServer.sin_addr); 
  // construct nonce
  
  char buffer[1500]; 
  bzero(buffer, 1500); 
  
  uint16_t hnonce, nnonce; 
  hnonce = rand(); 

  nnonce = htons(hnonce); 
  size_t bpos = 0; 
  memcpy(&buffer[bpos], &nnonce, sizeof(nnonce)); 
  bpos += 2; 

  uint16_t hecnt, necnt; 
  hecnt = 1 ; 
  necnt = htons(hecnt); 
  memcpy(&buffer[bpos], &necnt, sizeof(necnt)); 
  bpos += sizeof(necnt); 

  int i; 

  // copy the addresses
  for (i = 0; i < 10; i++)
    {
      memcpy(&buffer[bpos], &addrs[i], sizeof(uint8_t)); 
      bpos += sizeof(uint8_t); 
    }
  
  // then the event data
  memcpy(&buffer[bpos], &e.cmd, 1); 
  bpos += 1; 
  memcpy(&buffer[bpos], &e.src, 1); 
  bpos += 1; 

  for (i = 0; i < 5; i++)
    {
      uint16_t nedata, hedata; 
      hedata = e.data[i]; 
      nedata = htons(hedata); 
      memcpy(&buffer[bpos], &nedata, sizeof(uint16_t)); 
      bpos += sizeof(uint16_t); 
    }
  bpos += 12;
  
  // single event
  printf("sending event\n"); 
  sendto(sock, buffer, bpos, 0, 
	 (struct sockaddr*)&saServer, sizeof(saServer)); 

  recv(sock, buffer, 1500, 0); 
  // extract out success/failure data
  //uint16_t success = buffer[2]; 
/*   printf("received response %d %d %d %d %d %d %d %d \n",  */
/* 	 (int)buffer[0],  */
/* 	 (int)buffer[1],  */
/* 	 (int)buffer[2],  */
/* 	 (int)buffer[3],  */
/* 	 (int)buffer[4],  */
/* 	 (int)buffer[5],  */
/* 	 (int)buffer[6],  */
/* 	 (int)buffer[7]);  */

  Py_RETURN_NONE;
 
}


static PyMemberDef PyNetEvent_members[] = {
    {"rxSet", T_OBJECT_EX, offsetof(PyNetEvent, rxSet), 0, 
     "receive set of (cmd, src) tuples"}, 
    {"number", T_INT, offsetof(PyNetEvent, number), 0,
     "pynetevent number"},
    {"somaIP", T_OBJECT_EX, offsetof(PyNetEvent, ip), 0, 
     "soma device IP address"}, 
    {NULL}  /* Sentinel */
};


static PyMethodDef PyNetEvent_methods[] = {
    {"startEventRX", (PyCFunction)PyNetEvent_startEventRX, METH_NOARGS, 
     "begin rx of event packets"}, 
    {"stopEventRX", (PyCFunction)PyNetEvent_stopEventRX, METH_NOARGS, 
     "stop rx of event packets"}, 
    {"sendEvent", (PyCFunction)PyNetEvent_send, METH_VARARGS, 
     "send event packet"}, 
    {"getEvents", (PyCFunction)PyNetEvent_getEvents, METH_NOARGS, 
     "Get the events, as "
     "a list of (cmd, src, data1, data2, data3, data4, data5)"}, 
    {NULL}  /* Sentinel */
};

static PyTypeObject PyNetEventType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pynetevent.PyNetEvent",             /*tp_name*/
    sizeof(PyNetEvent),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyNetEvent_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "PyNetEvent objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyNetEvent_methods,             /* tp_methods */
    PyNetEvent_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyNetEvent_init,      /* tp_init */
    0,                         /* tp_alloc */
    PyNetEvent_new,                 /* tp_new */
};

static PyMethodDef module_methods[] = {
    {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL importexport */
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC
initpynetevent(void) 
{
    PyObject* m;

    if (PyType_Ready(&PyNetEventType) < 0)
        return;

    m = Py_InitModule3("pynetevent", module_methods,
                       "Example module that creates an extension type.");


    if (m == NULL)
      return;

    Py_INCREF(&PyNetEventType);
    PyModule_AddObject(m, "PyNetEvent", (PyObject *)&PyNetEventType);
}



int setupRXSocket()
{

  int sock; 

  struct sockaddr_in si_me;
  
    
  sock = socket(AF_INET, SOCK_DGRAM, 17); 
  
  memset((char *) &si_me, sizeof(si_me), 0);

  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(EVENTRXPORT); 

  si_me.sin_addr.s_addr = INADDR_ANY; 
  
  int optval = 1; 

  // confiugre socket for reuse
  optval = 1; 
  int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 
	     &optval, sizeof (optval)); 

  optval = 4000000; 
  res = setsockopt (sock, SOL_SOCKET, SO_RCVBUF, 
		    (const void *) &optval, sizeof(optval)); 

  socklen_t optlen;   
  res = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, 
		   (void *) &optval, &optlen); 

  res =  bind(sock, (struct sockaddr*)&si_me, sizeof(si_me)); 

    
  return sock; 
}

uint32_t getEvents(int sock, char * rxValidLUT, 
		   struct EventList_t * eventlist)
{
  /*
    returns the sequence number for us to "deal with"
    and creates a 

  */ 
  const int EBUFSIZE = 1550; 
  char buffer[EBUFSIZE]; 

  struct sockaddr_in sfrom; 
  socklen_t fromlen = sizeof(sfrom); 
  
  size_t len = recvfrom(sock, buffer, EBUFSIZE, 
			0, (struct sockaddr*)&sfrom, &fromlen); 
  
  // extract out events
  
  uint32_t seq = ntohl(*((int *) &buffer[0])); 

  
  // decode the transmited event set into an array of events 
  
  size_t bpos = 4; 
  
  struct eventListItem_t * curelt = NULL;
  int addedcnt = 0; 

  while ((bpos+1) < len) 
    { 
      uint16_t neventlen, eventlen;
      memcpy(&neventlen, &buffer[bpos], sizeof(neventlen));
      eventlen = ntohs(neventlen);
      bpos += 2;
      int evtnum;
      for (evtnum = 0; evtnum < eventlen; evtnum++)
	{
	  // extract out individual events
	  struct event_t evt;
	  evt.cmd = buffer[bpos];
	  bpos++;
	  
	  evt.src = buffer[bpos];
	  bpos++;
	  
	  // we need to be much more careful about extracting out the 
	  // event data here
	  int i =0; 
	  for (i = 0; i < EVENTLEN-1; i++) {
	    uint16_t nedata, hedata; 
	    memcpy(&nedata, &buffer[bpos], sizeof(uint16_t)); 
	    hedata = ntohs(nedata); 
	    evt.data[i] = hedata; 
	    bpos += sizeof(uint16_t); 
	  }
	  
	  // now, is this one of ours?
 	  if (rxValidLUT[evt.cmd + evt.src*256] != 0) { 

 	    // now we add the thing 
	    struct eventListItem_t * newelt; 
	    newelt = malloc(sizeof(struct eventListItem_t)); 
	    bzero(newelt, sizeof(struct eventListItem_t)); 
	    newelt->elt = NULL; 
	    if (curelt == NULL ) {
	      eventlist->eltHead = newelt; 
	      eventlist->eltTail = newelt; 
	      curelt = newelt; 
	    } else {
	      curelt->elt = newelt; 
	      curelt = newelt; 
	      
	      eventlist->eltTail = curelt; 
	      
	    }
	    addedcnt += 1; 
	    curelt->e = evt;
	    //printf("Added element %d\n", curelt->elt ); 
 	  } 
	  
	}
    }
  eventlist->size = addedcnt; 

  return seq;
  
}


