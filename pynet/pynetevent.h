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
#include "netevent2.h"

typedef struct {
  PyObject_HEAD
  PyObject *rxSet; /* RX set list */ 
  PyObject *ip; /* Soma device IP address */ 
  struct NetEvent_Handle * nethandle;  

} PyNetEvent;

#endif //PYNETEVENT_H
