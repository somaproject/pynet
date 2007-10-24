import pynetevent
import time
import numpy


p = pynetevent.PyNetEvent()

for i in range(255):
    p.rxSet.add((100, i))

p.startEventRX()
time.sleep(1)
for i in range(10):
    p.sendEvent((0x08, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0x00), 
                (100, 200, i, 3, 4, 5, 6))
data = []
#time.sleep(2)
print "now going to get 10"
# we expect to get 10
numrec = 0
while (numrec < 10):
    rxdata = p.getEvents()
    data.extend(rxdata)
    numrec += len(rxdata)
    
# build event histogram
eventhist = {}
for i in data:
    cmd = i[0]
    if cmd in eventhist:
        eventhist[cmd] += 1
    else:
        eventhist[cmd] = 1

print eventhist
    
