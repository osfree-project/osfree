# Makefile for FIND (using Borland C or Turbo C)
# Based on the CHOICE makefile by Tom Ehlert

# set 
#   UPX=-rem 
# if you dont want to UPX choice
# if you use upx: --8086 for 8086 compatibility, --best for smallest

# UPX=-rem
UPX=upx --8086 --best

############# WATCOM ########################
# CC=wcl
# CFLAGS=-oas -s -wx -we -zq -fm -fe=

############# TURBO_C ########################
# -w warn -M create map -f- no floating point -Z register optimize
# -O jump optimize -k- no standard stack frome -K unsigned char
# -exxx executable name (must be last) -mt tiny (default is small)
# -N stack checking -a- byte alignment  -ln no default libs
# -lt create .com file -lx no map file ...
CC=tcc
CFLAGS=-w -M -f- -Z -a- -O -k- -K -mt -ln -lt -e

CFILES=find.c find_str.c lfnapi.c

# prf.c talloc.c tcdummy.c

# targets:

all: find.com

# no space between -e and find (set exe file name)!
# upx either find.com or find.exe, depends...

find.com: $(CFILES) kitten.c io95\io95.lib lfnchk.obj makefile
   $(CC) $(CFLAGS)find $(CFILES) kitten.c lfnchk.obj io95\io95.lib
   $(UPX) find.com

lfnchk.obj: lfnchk.asm
   nasm lfnchk.asm -f obj -o lfnchk.obj

io95\io95.lib:
   @cd io95
   @make
   @cd ..

clean:
   -del *.obj
   -cd io95
   -make clean
   -cd ..

