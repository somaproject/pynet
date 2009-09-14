#include "pynetevent.h"

static void
PyNetEvent_dealloc(PyNetEvent* self)
{
    self->ob_type->tp_free((PyObject*)self);
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
	
	self->nethandle = NetEvent_new(addrstr); 

    }

    return (PyObject *)self;
}

static int
PyNetEvent_init(PyNetEvent *self, PyObject *args, PyObject *kwds)
{
    return 0;
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
  NetEvent_resetMask(self->nethandle); 

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
      
      NetEvent_setMask(self->nethandle, src, cmd); 
      
    } else {
      // error
      Py_RETURN_NONE; 
    }
    /* release reference when done */
    Py_DECREF(item);
  }

  NetEvent_startEventRX(self->nethandle); 

  Py_DECREF(iterator);
  
  Py_RETURN_NONE; 
}

PyObject * eventToPyTuple(struct NetEvent_event_t * evt)
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

  

  Py_RETURN_NONE; 

}

static PyObject *
PyNetEvent_getEvents(PyNetEvent* self,  PyObject *args)
{


  
  int blocking = 0; 

  if (!PyArg_ParseTuple(args, "b", &blocking))
        return NULL;
  
  int netevent_count = 10; 
  struct NetEvent_event_t * events = calloc(netevent_count, sizeof(struct NetEvent_event_t) * netevent_count); 
  
  int ret = NetEvent_getEvents(self->nethandle, events, 
			       netevent_count, blocking); 
  if (ret <= 0 ) {
      Py_INCREF(Py_None);
      return Py_None; 
  }

  PyObject * outlist = PyList_New(0); 
  int i; 
  for (i = 0; i < ret; i++) {
    PyObject * outtuple = eventToPyTuple(&events[i]);     
    PyList_Append(outlist, outtuple); 
    Py_DECREF(outtuple); 
  }
  

  free(events); 
  
  return outlist; 

  
}

static PyObject*
PyNetEvent_send(PyNetEvent* self, PyObject *args, PyObject *kwds)
{
  
  struct NetEvent_event_t e; 
  uint8_t addrs[10]; 
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

  NetEvent_sendEvent(self->nethandle, &e, addrs); 

  Py_RETURN_NONE;
 
}


static PyMemberDef PyNetEvent_members[] = {
    {"rxSet", T_OBJECT_EX, offsetof(PyNetEvent, rxSet), 0, 
     "receive set of (cmd, src) tuples"}, 
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
    {"getEvents", (PyCFunction)PyNetEvent_getEvents, METH_VARARGS, 
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



