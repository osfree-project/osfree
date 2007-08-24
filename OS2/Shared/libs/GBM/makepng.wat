# Makefile for libpng (static)

# This makefile is for Open Watcom C++ on OS/2.
# It builds the libpng library as a statically
# linkable library (.LIB).

# Modify this line to point to the zlib library 1.2.3 or higher
ZLIB = ..\zlib.123

# set directory where binaries will be placed by target install
IDIR = bin

#
# Using Open Watcom C++:
#
CC      = wcc386
CLIB    = wlib

# Builds libpng objects which are compiled multithreaded
# Therefore all users should also be multithreaded
CWARNS     = -wx -wcd=124 -wcd=136 -we
CFLAGS     = $(CWARNS) -i$(ZLIB) -bd -zq -zp4 -onatxh -oe=50 -sg -ei -6r -fp5 -bm -mf /DPNG_NO_STDIO /DNDEBUG
CFLAGS_DBG = $(CWARNS) -i$(ZLIB) -bd -zq -zp4 -sg -ei -6r -fp5 -bm -mf -d2 /DDEBUG

LFLAGS     = SYSTEM os2v2_dll op q op el op stack=0x6000 op caseexact
LFLAGS_DBG = SYSTEM os2v2_dll op q d all op stack=0x6000 op caseexact

LIBFLAGS     = -q -n -b -c

# For debugging call nmake or wmake with debug=on
!ifdef debug
CFLAGS   = $(CFLAGS_DBG)
LFLAGS   = $(LFLAGS_DBG)
!endif

.SUFFIXES:	.c .obj

.c.obj:
		$(CC) $(CFLAGS) $*.c

#

OBJS = png.obj pngset.obj pngget.obj pngrutil.obj pngtrans.obj pngwutil.obj \
       pngread.obj pngrio.obj pngwio.obj pngwrite.obj pngrtran.obj          \
       pngwtran.obj pngmem.obj pngerror.obj pngpread.obj

all:   png.lib

png.lib:  $(OBJS)
          $(CLIB) $(LIBFLAGS) $@ +png.obj +pngset.obj +pngget.obj +pngrutil.obj +pngtrans.obj +pngwutil.obj\
                                 +pngread.obj +pngrio.obj +pngwio.obj +pngwrite.obj +pngrtran.obj\
                                 +pngwtran.obj +pngmem.obj +pngerror.obj +pngpread.obj

png.obj:  png.c png.h pngconf.h
          $(CC) $(CFLAGS) $*.c


# ------------------------
# Build management targets
# ------------------------

clean:
         -del /N *.obj *.lst 2> nul

clobber:
         -del /N *.obj *.lst *.exe *.lib $(IDIR)\* 2> nul

install:
         copy png.lib     $(IDIR)
         copy png.h       $(IDIR)
         copy pngconf.h   $(IDIR)

package:
         -del /N $(IDIR)\libpng1214.zip
         zip -9 $(IDIR)\libpng1214.zip png.lib png.h pngconf.h

