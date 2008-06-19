from somapynet.event  import Event
from somapynet.neteventio import NetEventIO	
from somapynet import eaddr

eio = NetEventIO("10.0.0.2")

pingresponse = 0x09
eio.addRXMask(pingresponse, eaddr.SYSCONTROL)

eio.start()

pingrequest = 0x08
e = Event()
e.src = eaddr.NETWORK
e.cmd = pingrequest
e.data[4] = 0x1234

ea = eaddr.TXDest()
ea[eaddr.SYSCONTROL] = 1

eio.sendEvent(ea, e)

erx = eio.getEvents()
for event in erx:
  print event

eio.stop()
