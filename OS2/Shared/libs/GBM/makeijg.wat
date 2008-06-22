# Makefile for Independent JPEG Group's software

# This makefile is for Open Watcom C++ on OS/2.
# It builds the IJG library as a statically linkable library (.LIB).
#
# Stripped down makefile for compiled the subset of IJG 6a/6b used by
# the OS/2 version of the Generalised Bitmap Module.

# set directory where binaries will be placed by target install
IDIR = bin

#
# Using Open Watcom C++:
#
CC = wcc386

# Compile objects suitable for multithreaded GBM.DLL
CWARNS         = -wx -wcd=136 -wcd=201 -we -za
CFLAGS         = $(CWARNS) -bd -zq -zp4 -onatxh -oe=50 -sg -ei -6r -fp6 -fpi87 -bm -mf /DNDEBUG
CFLAGS_DBG     = $(CWARNS) -bd -zq -zp4 -sg -ei -6r -fp6 -fpi87 -bm -mf -d2 /DDEBUG
LIBFLAGS       = -q -n -b -c

# For debugging call nmake or wmake with debug=on
!ifdef debug
CFLAGS   = $(CFLAGS_DBG)
!endif

# Put here the object file name for the correct system-dependent memory
# manager file.  For NT we suggest jmemnobs.obj, which expects the OS to
# provide adequate virtual memory.

# Use virtual memory rather than named temporary files
SYSDEPMEM =	jmemnobs.obj

# End of configurable options.

# source files: JPEG library proper
LIBSOURCES= jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
        jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c \
        jcphuff.c jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c \
        jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c \
        jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c \
        jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c \
        jfdctint.c jidctflt.c jidctfst.c jidctint.c jidctred.c jquant1.c \
        jquant2.c jutils.c jmemmgr.c

# memmgr back ends: compile only one of these into a working library
SYSDEPSOURCES= jmemansi.c jmemname.c jmemnobs.c jmemdos.c jmemmac.c

SOURCES= $(LIBSOURCES) $(SYSDEPSOURCES) $(APPSOURCES)

# files included by source files
INCLUDES= jchuff.h jdhuff.h jdct.h jerror.h jinclude.h jmemsys.h jmorecfg.h \
        jpegint.h jpeglib.h jversion.h cdjpeg.h cderror.h
# library object files common to compression and decompression
COMOBJECTS= jcomapi.obj jutils.obj jerror.obj jmemmgr.obj $(SYSDEPMEM)

# compression library object files
CLIBOBJECTS= jcapimin.obj jcapistd.obj jctrans.obj jcparam.obj jdatadst.obj jcinit.obj jcmaster.obj jcmarker.obj jcmainct.obj jcprepct.obj jccoefct.obj \
jccolor.obj jcsample.obj jchuff.obj jcphuff.obj jcdctmgr.obj jfdctfst.obj jfdctflt.obj jfdctint.obj

# decompression library object files
DLIBOBJECTS= jdapimin.obj jdapistd.obj jdtrans.obj jdatasrc.obj jdmaster.obj jdinput.obj jdmarker.obj jdhuff.obj jdphuff.obj jdmainct.obj jdcoefct.obj \
jdpostct.obj jddctmgr.obj jidctfst.obj jidctflt.obj jidctint.obj jidctred.obj  jdsample.obj jdcolor.obj jquant1.obj jquant2.obj jdmerge.obj

# These objectfiles are included in libjpeg.a
LIBOBJECTS= $(CLIBOBJECTS) $(DLIBOBJECTS) $(COMOBJECTS)


# Template command for compiling .c to .obj
.c.obj :
	$(CC) $(CFLAGS) $*.c


all: jconfig.h jpeg.lib

jconfig.h: jconfig.doc
	echo You must prepare a system-dependent jconfig.h file.
	echo Please read the installation directions in install.doc.
	exit 1

jpeg.lib: $(LIBOBJECTS)
           wlib $(LIBFLAGS) $@ +jmemnobs.obj \
                            +jcomapi.obj  +jutils.obj   +jerror.obj   +jmemmgr.obj \
                            +jcapimin.obj +jcapistd.obj +jctrans.obj  +jcparam.obj  +jdatadst.obj +jcinit.obj   +jcmaster.obj +jcmarker.obj +jcmainct.obj +jcprepct.obj +jccoefct.obj \
                            +jccolor.obj  +jcsample.obj +jchuff.obj   +jcphuff.obj  +jcdctmgr.obj +jfdctfst.obj +jfdctflt.obj +jfdctint.obj \
                            +jdapimin.obj +jdapistd.obj +jdtrans.obj  +jdatasrc.obj +jdmaster.obj +jdinput.obj  +jdmarker.obj +jdhuff.obj   +jdphuff.obj  +jdmainct.obj +jdcoefct.obj \
                            +jdpostct.obj +jddctmgr.obj +jidctfst.obj +jidctflt.obj +jidctint.obj +jidctred.obj +jdsample.obj +jdcolor.obj  +jquant1.obj  +jquant2.obj  +jdmerge.obj


# ------------------------
# Build management targets
# ------------------------

clean:
	 -del /N $(LIBOBJECTS) *.lst 2> nul

clobber:
	 -del /N $(LIBOBJECTS) *.lst jpeg.lib 2> nul

install:
         copy jpeg.lib     $(IDIR)
         copy jpeglib.h    $(IDIR)
         copy jconfig.h    $(IDIR)
         copy jerror.h     $(IDIR)
         copy jmorecfg.h   $(IDIR)
         copy jpegint.h    $(IDIR)

package:
         -del /N $(IDIR)\libjpeg.zip
         zip -9 $(IDIR)\libjpeg.zip jpeg.lib jpeglib.h, jerror.h, jconfig.h, jmorecfg.h jpegint.h


jcapimin.obj: jcapimin.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcapistd.obj: jcapistd.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jccoefct.obj: jccoefct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jccolor.obj: jccolor.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcdctmgr.obj: jcdctmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jchuff.obj: jchuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jchuff.h
jcinit.obj: jcinit.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcmainct.obj: jcmainct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcmarker.obj: jcmarker.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcmaster.obj: jcmaster.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcomapi.obj: jcomapi.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcparam.obj: jcparam.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcphuff.obj: jcphuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jchuff.h
jcprepct.obj: jcprepct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jcsample.obj: jcsample.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jctrans.obj: jctrans.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdapimin.obj: jdapimin.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdapistd.obj: jdapistd.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdatadst.obj: jdatadst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h
jdatasrc.obj: jdatasrc.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h
jdcoefct.obj: jdcoefct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdcolor.obj: jdcolor.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jddctmgr.obj: jddctmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jdhuff.obj: jdhuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdhuff.h
jdinput.obj: jdinput.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmainct.obj: jdmainct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmarker.obj: jdmarker.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmaster.obj: jdmaster.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmerge.obj: jdmerge.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdphuff.obj: jdphuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdhuff.h
jdpostct.obj: jdpostct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdsample.obj: jdsample.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdtrans.obj: jdtrans.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jerror.obj: jerror.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jversion.h jerror.h
jfdctflt.obj: jfdctflt.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jfdctfst.obj: jfdctfst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jfdctint.obj: jfdctint.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jidctflt.obj: jidctflt.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jidctfst.obj: jidctfst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jidctint.obj: jidctint.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jidctred.obj: jidctred.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jquant1.obj: jquant1.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jquant2.obj: jquant2.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jutils.obj: jutils.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jmemmgr.obj: jmemmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
jmemansi.obj: jmemansi.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
jmemname.obj: jmemname.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
jmemnobs.obj: jmemnobs.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
jmemdos.obj: jmemdos.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
jmemmac.obj: jmemmac.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
