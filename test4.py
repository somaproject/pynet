import pynetevent
import time
import numpy
import sys

p = pynetevent.PyNetEvent("10.0.0.3")

for i in range(255):
    p.rxSet.add((130, i))
    

p.startEventRX()
print "EventRX started" 

# note that this is sending an event to the acqboard, so to get
# a response back, we'll need to change ID on each run


ID = int(sys.argv[1])
CMD = 1
p.sendEvent((0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0xFF, 0xFF),
            (0x50, 4, (ID << 8) | (CMD & 0xF), 3, 4, 5, 6))

print "Event sent" 
rxdata = p.getEvents()
print rxdata

