
import sys
sys.path.append("../")

from somapynet.event import Event
from somapynet import eaddr
from somapynet.neteventio import NetEventIO

eio = NetEventIO("10.0.0.2")

eio.addRXMask(xrange(256), eaddr.NETWORK)

eio.start()

e = Event()
for i in xrange(4):
    e.src = eaddr.NETWORK
    e.cmd = 0x20
    e.data[0] = 0x0123
    e.data[1] = 0x4564
    e.data[2] = 0x1122
    e.data[3] = 0x89AB
    e.data[4] = i 
    
    ea = eaddr.TXDest()
    ea[eaddr.NETWORK] = 1
    
    eio.sendEvent(ea, e)
    

events = []
while len(events) < 4:
    erx = eio.getEvents()
    events = events + erx
    
for e in events:
    print e
eio.stop()


