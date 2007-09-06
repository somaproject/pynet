import numpy

def addrToTuple(x):
    """
    Take in an 80-bit address boolean
    and converts it to a sixteen-bit tuple

    """

    words = []
    for i in range(5):
        highbyte = 0
        lowbyte = 0
        for j in range(8, 0, -1):
            highbyte = (highbyte << 1)
            if x[i*16 + j] > 0:
                highbyte |= 1
                
            lowbyte = (lowbyte << 1)
            if x[i*16 + j+7] > 0:
                lowbyte |= 1
        print hex(highbyte), hex(lowbyte)        
        words.append((lowbyte << 8) | highbyte)

    return words

def test():
    x = numpy.zeros(80, dtype = numpy.uint8)
    x[3] = 1
    print [hex(i) for i in addrToTuple(x)]

if __name__ == "__main__":
    test()
    
