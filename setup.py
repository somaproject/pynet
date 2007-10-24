#/usr/bin/env python
from distutils.core import setup, Extension

setup(name="pynetevent", version="1.0",
      ext_modules=[Extension("pynetevent", ["pynetevent.c"])])
