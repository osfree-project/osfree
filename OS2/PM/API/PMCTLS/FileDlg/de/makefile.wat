#
# GBM enhanced file dialog (German)
#
# This makefile is for Open Watcom C on OS/2.
#
# Builds GBMDLG.DLL which is compiled multithreaded
# Therefore all users should also be multithreaded
#

# include configuration setting for nmake (except compiler options)
!INCLUDE ..\..\nmake.opt

GBM  = ..\..\gbmos2
IDIR = ..\bin

# ------------------
# Configure compiler
# ------------------

#
# Using Open Watcom C:
#
CC      = wcc386
CL      = wlink
CLIB    = wlib
RC      = wrc

# Builds gbm objects which are compiled multithreaded
# Therefore all users should also be multithreaded
CWARNS      = -wx -we
CFLAGS      = $(CWARNS) -i$(GBM) -bd -zq -zp4 -onatxh -sg -ei -6r -fp5 -bm -mf /DNDEBUG
CFLAGS_DBG  = $(CWARNS) -i$(GBM) -bd -zq -zp4 -sg -ei -6r -fp5 -bm -mf -d2 /DDEBUG

LFLAGS      = sys os2v2_dll initi termi op many op caseexact op q op el
LFLAGS_DBG  = sys os2v2_dll initi termi op many op caseexact op q d all

LIBFLAGS    = -q -n -b -c

# For debugging call nmake or wmake with debug=on
!ifdef debug
CFLAGS     = $(CFLAGS_DBG)
LFLAGS     = $(LFLAGS_DBG)
!endif

#

.SUFFIXES:	.c .obj

.c.obj:
		$(CC) $(CFLAGS) $*.c

#

all:		gbmdlg.dll gbmdlg.lib gbmdlg.hlp

#

gbmdlg.dll:	gbmdlg.obj $(GBM)\gbm.lib gbmdlg.lnk gbmdlg.res
                $(CL) $(LFLAGS) @gbmdlg name $@ file gbmdlg.obj library $(GBM)\gbm.lib,$(GBM)\gbmscale.lib
		rc -n -x2 gbmdlg.res $@

gbmdlg.lib:	gbmdlg.lnk
                $(CLIB) $(LIBFLAGS) $@ +gbmdlg.dll

gbmdlg.obj:	gbmdlg.c gbmdlg.h gbmdlgrc.h $(GBM)\gbm.h $(GBM)\gbmscale.h

gbmdlg.res:	gbmdlg.rc gbmdlgrc.h
		rc -n -r -x2 gbmdlg.rc

gbmdlg.hlp:	gbmdlg.scr gbmdlgrc.h
		ipfcprep gbmdlg.scr gbmdlg.ipf
		ipfc gbmdlg.ipf /country=049 /codepage=850 /language=DEU /X /W3

#

clean:
		-del /N *.obj *.res *.ipf 2> nul

clobber:
		-del /N *.dll *.lib *.obj *.res *.ipf *.hlp 2> nul
                -del /N $(IDIR)\gbmdlg.dll $(IDIR)\gbmdlg.hlp $(IDIR)\gbmdlg.h $(IDIR)\gbmdlg.lib 2> nul

#

install:
                copy gbmdlg.dll $(IDIR)
                copy gbmdlg.hlp $(IDIR)
                copy gbmdlg.h   $(IDIR)
                copy gbmdlg.lib $(IDIR)

#

exepack:
         $(DLL_PACKER) *.dll

#

package:
        -del /N $(IDIR)\gbmdlg_dll.zip 2> nul
        zip -9 $(IDIR)\gbmdlg_dll.zip  gbmdlg.dll gbmdlg.hlp gbmdlg.h gbmdlg.lib

