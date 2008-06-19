from somapynet import eaddr 
from somapynet.neteventio import NetEventIO	

eio = NetEventIO("10.0.0.2")

timerid = 0
timecmd = 0x10

eio.addRXMask(timecmd, timerid)

eio.start()

erx = eio.getEvents()
for event in erx:
  print event

eio.stop()
