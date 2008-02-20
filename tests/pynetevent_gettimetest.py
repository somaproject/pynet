import pynetevent
import time
import numpy

"""
A simple event to read time events and guarantee we're
doing our byte-order operations correctly.
"""

p = pynetevent.PyNetEvent("10.0.0.2")

for i in range(255):
    p.rxSet.add((16, i))

p.startEventRX()

print "now going to get 10"
# we expect to get 10
data = []
numrec = 0
while (numrec < 100):
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

# these should be consecutive

for e in data:
    d0 = e[2]
    d1 = e[3]
    d2 = e[4]
    print (d0 << 32 | d1 << 16 | d2)
