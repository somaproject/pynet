import pynetevent
import time
import numpy
import sys

p = pynetevent.PyNetEvent("10.0.0.2")

for i in range(255):
    p.rxSet.add((130, i))
    

p.startEventRX()
print "EventRX started" 

# note that this is sending an event to the acqboard, so to get
# a response back, we'll need to change ID on each run

ID = int(sys.argv[1])
CMD = 1
addrtuple = (0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0xFF, 0xFF)
cmdtuple = (0x50, 4, (CMD << 8) | (ID & 0xF),
           0x1234, 0x5678, 14, 15)
print addrtuple
print cmdtuple

p.sendEvent(addrtuple, cmdtuple)


print "Event sent" 
rxdata = p.getEvents()
print rxdata

