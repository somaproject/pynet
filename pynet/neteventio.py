#!/usr/bin/python
"""
Higher-level interface to our sadly-named pynetevent module


"""

import pynetevent
from event import Event
import eaddr

class NetEventIO(object):
    def __init__(self, IPAddr):
        """
        IPAddr == target soma IP address
        """
        self.ipaddr = IPAddr
        
        self.pne = pynetevent.PyNetEvent(self.ipaddr)

    def addRXMask(self, cmds, srcs):
        """
        Add the cartesian product of cmd and src

        cmd and src can also be ints < 256, but in general are iterable
        objects

        
        """
        
        if hasattr(cmds, "__iter__"): 
            cmditer = cmds
        else:
            cmditer = [cmds]

        if hasattr(srcs, "__iter__"): 
            srciter = srcs
        else:
            srciter = [srcs]

        
        for cmd in cmditer:
            for src in srciter:
                self.pne.rxSet.add((cmd, src))
        
    def clearRXMask(self):
        self.pne.rxSet.clear()

    def start(self):
        """
        Begin RX of events
        """
        
        self.pne.startEventRX()

    def stop(self):
        """
        Begin RX of events
        """
        
        self.pne.stopEventRX()

    def sendEvent(self, edestaddr, event):
        """
        Send the event to the edestaddr
        """
        assert isinstance(edestaddr, eaddr.TXDest)
        assert isinstance(event, Event)
        
         
        self.pne.sendEvent(edestaddr.toTuple(), event.toTuple())

    def getEvents(self):
        eventout = []
        events = None
        while events == None:
            events =  self.pne.getEvents()
            
        for et in events:
            e = Event()
            e.loadTuple(et)
            eventout.append(e)
        return eventout
        
        
