#/usr/bin/env python
from distutils.core import setup, Extension

setup(name="somapynet",
      description = "Soma Python  network interface",
      author = "Eric Jonas",
      author_email = "jonas@mit.edu", 
      version="1.0",
      package_dir = {'somapynet': 'pynet'},                      
      packages = ['somapynet'], 
      ext_modules=[Extension("somapynet.pynetevent", ["pynet/pynetevent.c"])])
