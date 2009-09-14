from somapynet import pynetevent
import time
import numpy

"""
A simple event to read time events and guarantee we're
doing our byte-order operations correctly.
"""

p = pynetevent.PyNetEvent("10.0.0.2")

p.rxSet.add((16, 0))

p.startEventRX()


# we expect to get 10
data = []
numrec = 0
while (numrec < 100):
    rxdata = None
    while rxdata == None:
        print "calling getEvents"
        rxdata = p.getEvents(True)
        print "rx data = ", rxdata
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
p.stopEventRX()
oldx = 0
pos = 0
for e in data:
    d0 = e[2]
    d1 = e[3]
    d2 = e[4]
    x =  (d0 << 32 | d1 << 16 | d2)
    if x != oldx + 1 and pos != 0:
        print "OMG ERROR"
        print x, oldx

    oldx = x
    pos += 1
