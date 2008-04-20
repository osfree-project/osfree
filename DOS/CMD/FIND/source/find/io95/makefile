#
# Makefile for io95 by Steffen Kaiser
#

compiler = tcc -c
linker   = tcc
lib      = tlib

options = -v -y -vi # from the mklib.bat file

objects = ren95.obj mkdir95.obj l2s95.obj win95api.obj fopen95.obj \
     getcwd95.obj dir95.obj fllpth95.obj find95.obj open95.obj \
     dir95.obj

all: $(objects)
   -@del   io95.lib
   @$(lib) io95 + ren95.obj
   @$(lib) io95 + mkdir95.obj
   @$(lib) io95 + l2s95.obj
   @$(lib) io95 + win95api.obj
   @$(lib) io95 + fopen95.obj
   @$(lib) io95 + getcwd95.obj
   @$(lib) io95 + dir95.obj
   @$(lib) io95 + fllpth95.obj
   @$(lib) io95 + find95.obj
   @$(lib) io95 + open95.obj

ren95.obj: ren95.c
   @$(compiler) $(options) ren95.c

mkdir95.obj: mkdir95.c
   @$(compiler) $(options) mkdir95.c

l2s95.obj: l2s95.c
   @$(compiler) $(options) l2s95.c

win95api.obj: win95api.c
   @$(compiler) $(options) win95api.c

fopen95.obj: fopen95.c
   @$(compiler) $(options) fopen95.c

getcwd95.obj: getcwd95.c
   @$(compiler) $(options) getcwd95.c

dir95.obj: dir95.c
   @$(compiler) $(options) dir95.c

fllpth95.obj: fllpth95.c
   @$(compiler) $(options) fllpth95.c

find95.obj: find95.c
   @$(compiler) $(options) find95.c

open95.obj: open95.c
   @$(compiler) $(options) open95.c

clean:
   -@del ren95.obj
   -@del mkdir95.obj
   -@del l2s95.obj
   -@del win95api.obj
   -@del fopen95.obj
   -@del getcwd95.obj
   -@del dir95.obj
   -@del fllpth95.obj
   -@del find95.obj
   -@del open95.obj
   -@del io95.lib
