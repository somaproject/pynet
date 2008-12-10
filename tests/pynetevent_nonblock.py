from somapynet import pynetevent
import time
import numpy

"""
A simple event to read time events and guarantee we're
doing our byte-order operations correctly.
"""

p = pynetevent.PyNetEvent("10.0.0.2")

# NO EVENTS means that we should never have any events available
# thus should always block
## for i in range(255):
##     p.rxSet.add((16, i))

p.startEventRX()


# now, make sure this doesn't block
for i in range(1000):
    p.getEvents(False)


