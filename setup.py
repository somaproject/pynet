#/usr/bin/env python
from distutils.core import setup, Extension
import os
from string import Template
import subprocess
import os

# this is a total hack, oh well
pkg_name = 'somanetevent-1.0'
p = subprocess.Popen("pkg-config --cflags %s" % pkg_name, shell=True,
                     stdout=subprocess.PIPE)
p.wait()
cflags = p.communicate()[0]
print 'cflags are', cflags

includedirs = []
for seg in cflags.split():
    if seg[0:2] == "-I":
        includedirs.append(seg[2:])

p = subprocess.Popen("pkg-config --libs %s" % pkg_name, shell=True,
                     stdout=subprocess.PIPE)
p.wait()
libstr = p.communicate()[0]

libdirs = []
libs = []

for seg in libstr.split():
    if seg[0:2] == "-L":
        libdirs.append(seg[2:])
    elif seg[0:2] == "-l":
        libs.append(seg[2:])


setup(name="somapynet",
      description = "Soma Python  network interface",
      author = "Eric Jonas",
      author_email = "jonas@mit.edu", 
      version="1.0",
      package_dir = {'somapynet': 'pynet'},                      
      packages = ['somapynet'], 
      ext_modules=[Extension("somapynet.pynetevent", ["pynet/pynetevent.c"],
                             include_dirs = includedirs, 
                             library_dirs = libdirs,
                             libraries=libs
                             )])
