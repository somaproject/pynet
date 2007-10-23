
PYNAME = `python -c "import sys; (a, b, c, t, n) = sys.version_info; print 'python%d.%d' % (a, b),"`


CFLAGS =  -fPIC -I/usr/include/$(PYNAME) -I/home/jonas/python/lib/python/numpy/core/include/ -pthread -O3 -g -pg 

LDFLAGS = -pthread -shared -I/usr/include/$(PYNAME) -L/usr/lib -lpython2.5  -g -pg  -lboost_python-gcc41-mt -lboost_thread-gcc41-mt

all: pynetevent.so

pynetevent.o: pynetevent.c
	gcc $(CFLAGS) -c pynetevent.c


pynetevent.so: pynetevent.o
	gcc pynetevent.o -o pynetevent.so $(LDFLAGS)

