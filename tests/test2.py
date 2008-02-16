import gc
#gc.disable()

data = []
for i in xrange(100000):

    shortdata = []
    for j in range(57):

        mytuple = (j, i+1, i+2, i+3, i+4, i+5, i+6)
        shortdata.append(mytuple)
    data.extend(shortdata)


print len(data)
    
