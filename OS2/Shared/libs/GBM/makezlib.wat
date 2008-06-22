# Makefile for Zlib (static + DLL)

# This makefile is for Open Watcom C++ on OS/2.
# It builds the Zlib library as a statically as well as a dynamically
# linkable library (.LIB).
#

# set directory where binaries will be placed by target install
IDIR = bin

#
# Using Open Watcom C++:
#
CC      = wcc386
CL      = wlink
CLIB    = wlib

# Builds zlib objects which are compiled multithreaded
# Therefore all users should also be multithreaded
CWARNS     = -wx -wcd=201 -we
CFLAGS     = $(CWARNS) -bd -zq -zp4 -onatxh -oe=50 -sg -ei -6r -fp6 -fpi87 -bm -mf /DNDEBUG
CFLAGS_DBG = $(CWARNS) -bd -zq -zp4 -sg -ei -6r -fp6 -fpi87 -bm -mf -d2 /DDEBUG

LFLAGS     = SYSTEM os2v2_dll op q op el op stack=0x6000 op caseexact
LFLAGS_DBG = SYSTEM os2v2_dll op q d all op stack=0x6000 op caseexact

LIBFLAGS       = -q -n -b -c

# For debugging call nmake or wmake with debug=on
!ifdef debug
CFLAGS   = $(CFLAGS_DBG)
LFLAGS   = $(LFLAGS_DBG)
!endif


.SUFFIXES:	.c .obj

.c.obj:
		$(CC) $(CFLAGS) $*.c

#

OBJS = adler32.obj compress.obj crc32.obj gzio.obj uncompr.obj deflate.obj trees.obj \
       zutil.obj inflate.obj infback.obj inftrees.obj inffast.obj


all:   z.lib z.dll zdll.lib

z.dll:  $(OBJS)
	$(CL) $(LFLAGS) @zlib name $@ file adler32.obj,compress.obj,crc32.obj,gzio.obj,uncompr.obj,\
                                           deflate.obj,trees.obj,zutil.obj,inflate.obj,infback.obj,\
                                           inftrees.obj,inffast.obj

zdll.lib:  zlib.lnk
           $(CLIB) $(LIBFLAGS) $@ +z.dll

z.lib:   $(OBJS)
         $(CLIB) $(LIBFLAGS) $@ +adler32.obj +compress.obj +crc32.obj +gzio.obj +uncompr.obj\
                                +deflate.obj +trees.obj +zutil.obj +inflate.obj +infback.obj\
                                +inftrees.obj +inffast.obj

z.obj:   zlib.h zconf.h trees.h deflate.h zutil.h \
         inftrees.h inffast.h inflate.h inffixed.h crc32.h
         $(CC) $(CFLAGS) $*.c


# ------------------------
# Build management targets
# ------------------------

clean:
		 -del /N *.obj *.lst 2> nul

clobber:
		 -del /N *.obj *.lst *.lib *.dll $(IDIR)\* 2> nul

install:
         copy z.dll     $(IDIR)
         copy z.lib     $(IDIR)
         copy zdll.lib  $(IDIR)
         copy zlib.h    $(IDIR)
         copy zconf.h   $(IDIR)

package:
         -del /N $(IDIR)\zlib.zip
         zip -9 $(IDIR)\zlib.zip z.dll z.lib zdll.lib zlib.h zconf.h

