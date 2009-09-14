#ifndef __SOMA_NETEVENT_H__
#define __SOMA_NETEVENT_H__
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>

static const int DATARETXPORT = 4400; 

static const int EVENTRXRETXPORT = 5500; // event retransmission requests

static const int EVENTTXPORT = 5100; // events from software to the soma bus

// FROM SOMA TO SOFTWARE
// These are ports that we, the software, listen on. Soma sets these as the 
// DESTINATION port in packets that it sends. 

static const int EVENTRXPORT = 5000; 

#define EVENTLEN 6
struct  NetEvent_event_t
{
  unsigned char cmd; 
  unsigned char src; 
  uint16_t data[EVENTLEN-1]; 
}; 

struct NetEvent_eventListItem_t; 

struct NetEvent_eventListItem_t
{
  struct NetEvent_event_t e; 
  struct NetEvent_eventListItem_t * elt; 
}; 


struct NetEvent_EventList_t
{
  struct NetEvent_eventListItem_t * eltHead; 
  struct NetEvent_eventListItem_t * eltTail; 
  pthread_mutex_t mutex;
  pthread_mutex_t size_mutex; 
  pthread_cond_t size_thold_cv; 
  int size; 
} ; 

struct NetEvent_NetworkSharedThreadState_t
{
  struct NetEvent_EventList_t * pel; 
  pthread_mutex_t running_mutex;
  int running; 
  char * rxValidLUT; 
}  ; 

#define IPLEN 100

struct NetEvent_Handle { 
  int number;
  struct NetEvent_NetworkSharedThreadState_t * pnss; 
  pthread_t * pNetworkThread; 
  int txsocket; 
  char ip[IPLEN];
  char * rxValidLUT; 
} ; 

int setupRXSocket(void); 

struct NetEvent_Handle * NetEvent_new(char * addrstr); 
void NetEvent_free(struct NetEvent_Handle *); 
void NetEvent_pthread_runner(struct NetEvent_NetworkSharedThreadState_t * pnss); 
void NetEvent_startEventRX(struct NetEvent_Handle * nh); 

void NetEvent_stopEventRX(struct NetEvent_Handle * nh); 
int NetEvent_getEvents(struct NetEvent_Handle * nh,  struct NetEvent_event_t * etgt, int MAXEVENTS, int blocking); 

int NetEvent_sendEvent(struct NetEvent_Handle * nh, struct NetEvent_event_t * e, uint8_t *  addrs); 

int NetEvent_setMask(struct NetEvent_Handle * nh, int src, int cmd); 
void NetEvent_resetMask(struct NetEvent_Handle * nh); 
int NetEvent_unsetMask(struct NetEvent_Handle * nh, int src, int cmd); 


#endif //__SOMA_NETEVENT_H__
