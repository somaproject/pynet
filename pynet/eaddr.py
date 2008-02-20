import numpy

class TXDest():
    N = 80
    def __init__(self):
        self.addr = numpy.zeros(self.N, dtype=bool)

    def __len__(self):
        return self.N

    def __getitem__(self, key):
        return self.addr.__getitem__(key)

    def __setitem__(self, key, value):
        return self.addr.__setitem__(key, value)

    def __iter__(self):
        return self.addr.__iter__()

    def toTuple(self):
        """
        Take in an 80-bit address boolean
        and converts it to a sixteen-bit tuple

        """
        x = self.addr
        words = []
        for i in range(10):
            byte = 0
            for j in range(7, -1, -1):
                byte = (byte << 1)
                if x[i*8 + j] > 0:
                    byte |= 1

            words.append(byte)

        return tuple(words)

# now the constants

TIMER = 0
SYSCONTROL = 1
BOOTSTORE = 2
NETWORK = 3
NETCONTROL = 4
JTAG = 7
DSP = []
for i in xrange(64):
    DSP.append(i + 8)



def test():
    x = TXDest()
    x[3] = 1
    x[79] = 1
    tupres = x.toTuple()
    assert tupres[0] == 8
    assert tupres[9] == 0x80

if __name__ == "__main__":
    test()
    
