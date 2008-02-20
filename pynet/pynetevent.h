#ifndef PYNETEVENT_H
#define PYNETEVENT_H
#include <Python.h>
#include "structmember.h"
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

const int DATARETXPORT = 4400; 

const int EVENTRXRETXPORT = 5500; // event retransmission requests

const int EVENTTXPORT = 5100; // events from software to the soma bus

// FROM SOMA TO SOFTWARE
// These are ports that we, the software, listen on. Soma sets these as the 
// DESTINATION port in packets that it sends. 

const int EVENTRXPORT = 5000; 

#define EVENTLEN 6
struct  event_t
{
  char cmd; 
  char src; 
  uint16_t data[EVENTLEN-1]; 
} ; 

struct eventListItem_t; 

struct  eventListItem_t
{
  struct event_t e; 
  struct eventListItem_t * elt; 
}; 


struct EventList_t
{
  struct eventListItem_t * eltHead; 
  struct eventListItem_t * eltTail; 
  pthread_mutex_t mutex;
  pthread_mutex_t size_mutex; 
  pthread_cond_t size_thold_cv; 
  int size; 
} ; 

struct NetworkSharedThreadState_t
{
  struct EventList_t * pel; 
  pthread_mutex_t running_mutex;
  int running; 
  char * rxValidLUT; 
}  ; 

typedef struct {
  PyObject_HEAD
  PyObject *rxSet; /* RX set list */ 
  PyObject *ip; /* Soma device IP address */ 
  int number;
  struct NetworkSharedThreadState_t * pnss; 
  pthread_t * pNetworkThread; 
} PyNetEvent;

int setupRXSocket(void); 
uint32_t getEvents(int sock, char * rxValidLUT, 
		   struct EventList_t * eventlist); 

#endif //PYNETEVENT_H
