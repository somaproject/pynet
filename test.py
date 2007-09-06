import pynetevent
import time
n = pynetevent.PyNetEvent()

print n.rxSet
for i in range(255):
    n.rxSet.append((16, i))

print n.startEventRX()
data = []
for i in xrange(100):
    x=  n.getEvents()
    data.extend(x)

print len(data)


n.stopEventRX()


n.rxSet = []
for i in range(255):
    n.rxSet.append((128, i))
    n.rxSet.append((129, i))

print n.startEventRX()
data = []
for i in xrange(100):
    x=  n.getEvents()
    data.extend(x)
print data

n.stopEventRX()


print len(data)
