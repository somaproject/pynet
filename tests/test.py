import pynetevent
import time
n = pynetevent.PyNetEvent()

print n.rxSet
for i in range(255):
    n.rxSet.add((16, i))

print n.startEventRX()
data = []
for i in xrange(100):
    x=  n.getEvents()
    data.extend(x)

print len(data)


n.stopEventRX()


n.rxSet = set()
for i in range(255):
    n.rxSet.add((128, i))
    n.rxSet.add((129, i))

print n.startEventRX()
data = []
for i in xrange(100):
    x=  n.getEvents()
    data.extend(x)
print data

n.stopEventRX()


print len(data)
