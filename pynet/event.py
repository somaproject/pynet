#!/usr/bin/python

"""
An abstraction around the Soma Events

"""

class Event(object):

    def __init__(self):
        """
        Create an empty event

        """
        self.cmd = 0
        self.src = 0
        self.data = [0, 0, 0, 0, 0]
        
    def loadTuple(self, tuple):
        """
        load tuple data into event
        """

        self.cmd = tuple[0]
        self.src = tuple[1]
        for i in xrange(5):
            self.data[i] = tuple[i+2]

    def toTuple(self):
        """
        create tuple for pynetevent tx
        """
        return (self.cmd, self.src, self.data[0],
                self.data[1], self.data[2], self.data[3], self.data[4])

    def __str__(self):
        return "cmd: %2.2X src: %2.2X %4.4X %4.4X %4.4X %4.4X %4.4X" % self.toTuple()
    

if __name__ == "__main__":
    e = Event()
    e.loadTuple((1, 2, 3, 4, 5, 6, 7))
    
    
