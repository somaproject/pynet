#/usr/bin/env python
from distutils.core import setup, Extension

setup(name="somapynet",
      description = "Soma Python  network interface",
      author = "Eric Jonas",
      author_email = "jonas@mit.edu", 
      version="1.0",
      py_modules = ['eaddr', 'event', 'neteventio'], 
      ext_modules=[Extension("pynetevent", ["pynetevent.c"])])
