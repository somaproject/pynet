import pynetevent
import time
import numpy


p = pynetevent.PyNetEvent()

for i in range(255):
    p.rxSet.append((130, i))
    

p.startEventRX()
time.sleep(1)

ID = 7
CMD = 1
p.sendEvent((0x00, 0x00, 0x00, 0x00, 0x00,
             0x00, 0x00, 0x00, 0xFF, 0xFF),
            (0x50, 4, (ID << 8) | (CMD & 0xF), 3, 4, 5, 6))

rxdata = p.getEvents()
print rxdata

