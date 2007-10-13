#
# Generalised Bitmap Module
#
# This makefile is for Open Watcom C++ on OS/2.
#
# Builds GBM.DLL which is compiled multithreaded
# Therefore all users should also be multithreaded

# include configuration setting for nmake (except compiler options)
#!INCLUDE ..\nmake.opt

# Modify this line to point to the Independant JPEG groups library 6a/6b
# If you haven't got it, comment out the line instead.
# (Keep the current makefile !)
IJG    = jpeg-6b

32_BITS = 

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk
!include $(%ROOT)/mk/all.mk

# Modify these 2 lines to point to the libpng library 1.2.14 or higher
# and the zlib 1.2.3 or higher.
# If you haven't got them, comment out the lines instead.
ZLIB   = zlib.123
LIBPNG = libpng.1214

# Modify this line to point to the libtiff library 3.8.2
# If you haven't got it, comment out the line instead.
#LIBTIFF = libtiff.382
LIBTIFF = ..$(SEP)libtiff

# Documentation directory
DOCDIR = doc

# Installation directory: gbm.dll, gbm.lib and gbm.h files are copied to this directory
IDIR =  bin

# -------------------
# Configure lib usage
# -------------------

#CJPEG    = /DENABLE_IJG
CJPEG    = -dENABLE_IJG
#LIB_IJG  = $(IJG)\jpeg.lib
LIB_IJG  = $(ROOT)$(SEP)lib$(SEP)jpeglib.lib

# ZLIB is required for LIBPNG
#CPNG     = /DENABLE_PNG
CPNG     = -dENABLE_PNG
#LIB_ZLIB = $(ZLIB)\z.lib
#LIB_PNG  = $(LIBPNG)\png.lib
#LIB_ZLIB = $(ROOT)$(SEP)lib$(SEP)zlib.lib
LIB_ZLIB = $(ROOT)$(SEP)tools$(SEP)shared$(SEP)zlib$(SEP)libz.lib
LIB_PNG  = $(ROOT)$(SEP)lib$(SEP)lpng.lib

# ZLIB and IJG are required for LIBTIFF
#CTIFF    = /DENABLE_TIF
CTIFF    = -dENABLE_TIF
#LIB_TIFF = $(LIBTIFF)\tiff.lib
LIB_TIFF = $(LIBTIFF)$(SEP)tiff.lib


# ------------------
# Configure compiler
# ------------------

#
# Using Open Watcom C++:
#
CC      = wcc386
CL      = wlink
CLIB    = wlib

# Builds gbm objects which are compiled multithreaded
# Therefore all users should also be multithreaded
CWARNS         = -wx -we
CFLAGS         = $(CWARNS) $(COPT) -fo=$^&.$(O) -zq -zp4 -onatxh -oe=80 -sg -ei -6r -fp5 -bm -mf -dNDEBUG
CFLAGS_DBG     = $(CWARNS) $(COPT) -fo=$^&.$(O) -zq -zp4 -sg -ei -6r -fp5 -bm -mf -d2 -dDEBUG
CFLAGS_EXE     = $(CFLAGS)
CFLAGS_EXE_DBG = $(CFLAGS_DBG)
CFLAGS_DLL     = -bd $(CFLAGS) $(CJPEG) $(CPNG) $(CTIFF)
CFLAGS_DLL_DBG = -bd $(CFLAGS_DBG) $(CJPEG) $(CPNG) $(CTIFF)

LFLAGS         = op q op el op stack=0x6000
LFLAGS_DBG     = op q d all op stack=0x6000
LFLAGS_EXE     = SYSTEM os2v2 $(LFLAGS)
LFLAGS_EXE_DBG = SYSTEM os2v2 $(LFLAGS_DBG)
LFLAGS_DLL     = sys os2v2_dll initi termi op many op caseexact $(LFLAGS)
LFLAGS_DLL_DBG = sys os2v2_dll initi termi op many op caseexact $(LFLAGS_DBG)

LIBFLAGS       = -q -n -b -c

# For debugging call nmake or wmake with debug=on
!ifdef debug
CFLAGS_EXE     = $(CFLAGS_EXE_DBG)
CFLAGS_DLL     = $(CFLAGS_DLL_DBG)
LFLAGS_EXE     = $(LFLAGS_EXE_DBG)
LFLAGS_DLL     = $(LFLAGS_DLL_DBG)
!endif

# Enable for Pentium profiling (also combined with debug above)
# For profiling call nmake or wmake with debug=on
!ifdef profile
CFLAGS_EXE     = $(CFLAGS_EXE) -et
CFLAGS_DLL     = $(CFLAGS_DLL) -et
!endif

#

#.SUFFIXES:	.c .obj

# Already declared in build for osFree
#.c.obj:
#		$(CC) $(CFLAGS_EXE) $*.c

#
# ijg png 

all:    prep tiff &
        gbmdllout &
        gbm.dll gbm.lib &
        gbmtoolsout &
        gbmhdr.exe &
        gbmmir.lib gbmref.exe &
        gbmrect.lib gbmsub.exe &
        gbmscale.lib gbmsize.exe &
        gbmerr.lib gbmtrunc.lib gbmht.lib gbmhist.lib gbmmcut.lib gbmbpp.exe &
        gbmcpal.exe  &
        gbmgamma.exe &
        gbmconv.exe &
        gbmver.exe

#

#prep:
#   @echo -----------------------------
#   @echo JPEG support, via IJG library
#   @echo -----------------------------
#   @$(CP) $(IJG)\jconfig.doc $(IJG)\jconfig.h
#   @$(CP) makeijg.wat $(IJG)
#   @$(CP) jmorecfg.h $(IJG)
#   @echo --------------------------------
#   @echo PNG support, via LIBPNG and ZLIB
#   @echo --------------------------------
#   @$(CP) makezlib.wat $(ZLIB)
#   @$(CP) makepng.wat  $(LIBPNG)
prep: .SYMBOLIC
   @echo ---------------------------------------
   @echo TIFF support, via LIBTIFF, IJG and ZLIB
   @echo ---------------------------------------
   @$(CP) maketif.wat $(LIBTIFF)
   @echo.

ijg:
   @echo ---------------------
   @echo Compiling IJG library
   @echo ---------------------
!ifdef debug
   (cd $(IJG) && wmake -c -ms -h -f makeijg.wat debug=yes)
!else
   (cd $(IJG) && wmake -c -ms -h -f makeijg.wat)
!endif
   @echo.

#

png:
   @echo ----------------------
   @echo Compiling ZLIB library
   @echo ----------------------
!ifdef debug
   (cd $(ZLIB)   && wmake -c -ms -h -f makezlib.wat debug=yes)
!else
   (cd $(ZLIB)   && wmake -c -ms -h -f makezlib.wat)
!endif
   @echo.
   @echo ------------------------
   @echo Compiling LIBPNG library
   @echo ------------------------
!ifdef debug
   (cd $(LIBPNG) && wmake -c -ms -h -f makepng.wat debug=yes)
!else
   (cd $(LIBPNG) && wmake -c -ms -h -f makepng.wat)
!endif
   @echo.

#

tiff: .SYMBOLIC
   @echo -------------------------
   @echo Compiling LIBTIFF library
   @echo -------------------------
!ifdef debug
   (cd $(LIBTIFF) && wmake -c -ms -h -f maketif.wat debug=yes)
!else
   (cd $(LIBTIFF) && wmake -h -f maketif.wat all)
!endif
   @echo.
# (cd $(LIBTIFF) && wmake -c -ms -h -f maketif.wat)
#

gbmdllout: .SYMBOLIC
   @echo ---------------------
   @echo Compiling GBM library
   @echo ---------------------

gbmtoolsout:
   @echo -------------------
   @echo Compiling GBM tools
   @echo -------------------

#

gbm.dll:  gbm.obj    gbmpbm.obj  gbmpgm.obj gbmppm.obj gbmpnm.obj  &
          gbmbmp.obj gbmtga.obj  gbmkps.obj gbmiax.obj gbmpcx.obj  &
          gbmtif.obj gbmlbm.obj  gbmvid.obj gbmgif.obj gbmxbm.obj  &
          gbmspr.obj gbmpsg.obj  gbmgem.obj gbmcvp.obj gbmjpg.obj  &
          gbmpng.obj gbmxpm.obj  gbmxpmcn.obj gbmhelp.obj gbmmap.obj
          $(CL) $(LFLAGS_DLL) @gbm name $@ file gbm.obj,gbmpbm.obj,gbmpgm.obj,gbmppm.obj,&
                                                gbmpnm.obj,gbmbmp.obj,gbmtga.obj,gbmkps.obj,&
                                                gbmiax.obj,gbmpcx.obj,gbmtif.obj,gbmlbm.obj,&
                                                gbmvid.obj,gbmgif.obj,gbmxbm.obj,gbmspr.obj,&
                                                gbmpsg.obj,gbmgem.obj,gbmcvp.obj,gbmjpg.obj,&
                                                gbmpng.obj,gbmxpm.obj,gbmxpmcn.obj,gbmhelp.obj,gbmmap.obj,&
                                                $(LIB_IJG),$(LIB_ZLIB),$(LIB_PNG),$(LIB_TIFF)

gbm.lib:  gbm.lnk
          $(CLIB) $(LIBFLAGS) $@ +gbm.dll

gbm.obj:  gbm.c # gbm.h gbmhelp.h gbmdesc.h gbmpbm.h gbmpgm.h gbmpnm.h gbmppm.h gbmbmp.h gbmtga.h gbmkps.h gbmiax.h gbmpcx.h gbmtif.h gbmlbm.h gbmvid.h gbmgif.h gbmxbm.h gbmspr.h gbmpsg.h gbmgem.h gbmcvp.h gbmjpg.h gbmpng.h gbmxpm.h gbmxpmcn.h
          $(CC) $(CFLAGS_DLL) $*.c

# ------------
# File formats
# ------------

gbmpbm.obj:	gbmpbm.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmpgm.obj:	gbmpgm.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmpnm.obj:	gbmpbm.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmppm.obj:	gbmppm.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmbmp.obj:	gbmbmp.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmtga.obj:	gbmtga.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmkps.obj:	gbmkps.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmiax.obj:	gbmiax.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmpcx.obj:	gbmpcx.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmlbm.obj:	gbmlbm.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmvid.obj:	gbmvid.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmgif.obj:	gbmgif.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmxbm.obj:	gbmxbm.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmxpm.obj:	gbmxpm.c # gbm.h gbmhelp.h gbmdesc.h gbmmap.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmxpmcn.obj:	gbmxpmcn.c # gbmxpmcn.h gbm.h gbmmap.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmspr.obj:	gbmspr.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmpsg.obj:	gbmpsg.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmgem.obj:	gbmgem.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmcvp.obj:	gbmcvp.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) $*.c

# ------------------

gbmhelp.obj:	gbmhelp.c # gbm.h
		$(CC) $(CFLAGS_DLL) $*.c

gbmmap.obj:	gbmmap.c # gbm.h gbmmap.h
		$(CC) $(CFLAGS_DLL) $*.c

# ------------------

gbmjpg.obj:	gbmjpg.c # gbm.h gbmhelp.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) -i=$(IJG) $*.c

gbmpng.obj:	gbmpng.c # gbm.h gbmhelp.h gbmmap.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) -i=$(LIBPNG) -i=$(ZLIB) $*.c

gbmtif.obj:	gbmtif.c # gbm.h gbmhelp.h gbmmap.h gbmdesc.h
		$(CC) $(CFLAGS_DLL) -i=$(IJG) -i=$(ZLIB) -i=$(LIBTIFF) $*.c


# ------------
# Bitmap tools
# ------------

gbmtool.obj:    gbmtool.c # gbmtool.h gbm.h

gbmtos2.obj:    gbmtos2.c # gbmtool.h gbm.h

gbmtool.lib:	gbmtool.obj gbmtos2.obj
                $(CLIB) $(LIBFLAGS) $@ +gbmtool.obj +gbmtos2.obj

#

gbmhdr.exe:	gbmhdr.obj gbm.lib gbmtool.lib
		$(CL) $(LFLAGS_EXE) name $@ file gbmhdr.obj library gbm.lib,gbmtool.lib

gbmhdr.obj:	gbmhdr.c # gbm.h

#

gbmmir.lib:	gbmmir.obj
                $(CLIB) $(LIBFLAGS) $@ +gbmmir.obj

gbmmir.obj:	gbmmir.c

gbmref.exe:	gbmref.obj gbm.lib gbmmir.lib gbmtool.lib
		$(CL) $(LFLAGS_EXE) name $@ file gbmref.obj library gbm.lib,gbmmir.lib,gbmtool.lib

gbmref.obj:	gbmref.c # gbm.h gbmmir.h

#

gbmrect.lib:	gbmrect.obj
                $(CLIB) $(LIBFLAGS) $@ +gbmrect.obj

gbmrect.obj:	gbmrect.c

gbmsub.exe:	gbmsub.obj gbm.lib gbmrect.lib gbmtool.lib
		$(CL) $(LFLAGS_EXE) name $@ file gbmsub.obj library gbm.lib,gbmrect.lib,gbmtool.lib

gbmsub.obj:	gbmsub.c # gbm.h gbmrect.h

#

gbmerr.lib:	gbmerr.obj
                $(CLIB) $(LIBFLAGS) $@ +gbmerr.obj

gbmerr.obj:	gbmerr.c

#

gbmscale.lib:	gbmscale.obj
                $(CLIB) $(LIBFLAGS) $@ +gbmscale.obj

gbmscale.obj:	gbmscale.c

gbmsize.exe:	gbmsize.obj gbm.lib gbmscale.lib gbmtool.lib
		$(CL) $(LFLAGS_EXE) name $@ file gbmsize.obj library gbm.lib,gbmscale.lib,gbmtool.lib

gbmsize.obj:	gbmsize.c gbm.h gbmscale.h

#

gbmtrunc.lib:	gbmtrunc.obj
                $(CLIB) $(LIBFLAGS) $@ +gbmtrunc.obj

gbmtrunc.obj:	gbmtrunc.c

#

gbmht.lib:	gbmht.obj
                $(CLIB) $(LIBFLAGS) $@ +gbmht.obj

gbmht.obj:	gbmht.c

#

gbmhist.lib:	gbmhist.obj
                $(CLIB) $(LIBFLAGS) $@ +gbmhist.obj

gbmhist.obj:	gbmhist.c

#

gbmmcut.lib:	gbmmcut.obj
                $(CLIB) $(LIBFLAGS) $@ +gbmmcut.obj

gbmmcut.obj:	gbmmcut.c

#

gbmbpp.exe:	gbmbpp.obj gbm.lib gbmerr.lib gbmtrunc.lib gbmht.lib gbmhist.lib gbmmcut.lib gbmtool.lib
		$(CL) $(LFLAGS_EXE) name $@ file    gbmbpp.obj \
                                            library gbm.lib,gbmerr.lib,gbmtrunc.lib,gbmht.lib,gbmhist.lib,gbmmcut.lib,gbmtool.lib

gbmbpp.obj:	gbmbpp.c gbm.h gbmerr.h gbmtrunc.h gbmht.h gbmhist.h gbmmcut.h

#

gbmcpal.exe:	gbmcpal.obj gbm.lib gbmhist.lib gbmmcut.lib gbmtool.lib
		$(CL) $(LFLAGS_EXE) name $@ file gbmcpal.obj library gbm.lib,gbmhist.lib,gbmmcut.lib,gbmtool.lib

gbmcpal.obj:	gbmcpal.c gbm.h gbmhist.h gbmmcut.h

#

gbmgamma.exe:	gbmgamma.obj gbm.lib gbmtool.lib
		$(CL) $(LFLAGS_EXE) name $@ file gbmgamma.obj library gbm.lib,gbmtool.lib

gbmgamma.obj:	gbmgamma.c gbm.h

#

gbmconv.exe:	gbmconv.obj gbm.lib gbmtool.lib
		$(CL) $(LFLAGS_EXE) name $@ file gbmconv.obj library gbm.lib,gbmtool.lib

gbmconv.obj:	gbmconv.c gbm.h

#
# On OS/2 we load gbm.dll dynamically, so we don't need gbm.lib.

gbmver.exe:	gbmver.obj
		$(CL) $(LFLAGS_EXE) name $@ file gbmver.obj

gbmver.obj:	gbmver.c gbm.h


# ------------------------
# Build management targets
# ------------------------

clean:  prep
	-del /N *.obj *.err *.prf 2> nul
	(cd $(IJG)     && wmake -c -ms -h -f makeijg.wat  clean)
	(cd $(ZLIB)    && wmake -c -ms -h -f makezlib.wat clean)
	(cd $(LIBPNG)  && wmake -c -ms -h -f makepng.wat  clean)
	(cd $(LIBTIFF) && wmake -c -ms -h -f maketif.wat  clean)

clobber: prep
	 -del /N *.obj *.err *.prf *.exe *.lib *.dll $(IDIR)\* 2> nul
 	 (cd $(IJG)     && wmake -c -ms -h -f makeijg.wat  clobber)
	 (cd $(ZLIB)    && wmake -c -ms -h -f makezlib.wat clobber)
	 (cd $(LIBPNG)  && wmake -c -ms -h -f makepng.wat  clobber)
	 (cd $(LIBTIFF) && wmake -c -ms -h -f maketif.wat  clobber)

#

install:
         copy gbm.h      $(IDIR)
         copy gbmerr.h   $(IDIR)
         copy gbmhist.h  $(IDIR)
         copy gbmht.h    $(IDIR)
         copy gbmmcut.h  $(IDIR)
         copy gbmmir.h   $(IDIR)
         copy gbmrect.h  $(IDIR)
         copy gbmscale.h $(IDIR)
         copy gbmtrunc.h $(IDIR)
         copy gbmtool.h  $(IDIR)
         copy *.lib      $(IDIR)
         copy *.exe      $(IDIR)
         copy *.dll      $(IDIR)
         copy $(DOCDIR)\readme_gbmos2.txt $(IDIR)

#

exepack:
         $(DLL_PACKER) *.dll
         $(EXE_PACKER) *.exe

#

package:
        -del /N $(IDIR)\gbmos2.zip 2> nul
        zip $(IDIR)\gbmos2.zip *.exe *.dll *.lib gbm.h gbmerr.h gbmhist.h  \
                               gbmht.h gbmmcut.h gbmmir.h gbmrect.h  \
                               gbmscale.h gbmtrunc.h gbmtool.h \
                               $(DOCDIR)\readme_gbmos2.txt

