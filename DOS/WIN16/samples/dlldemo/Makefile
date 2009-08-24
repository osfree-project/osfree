#####################################################################
#
#  @(#)makefile	1.4 
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#####################################################################

include $(TWINDIR)/Makefile.config

project		= dlldemo
components	= dlldemo.o dlldemo.res.o libbitmaps.$(DLL_EXT)
libraries	= libbitmaps.$(DLL_EXT) libdllres.$(DLL_EXT)
linklibs	= -lshell$(TWIN_APISIZE)
resource	= dlldemo
resource_depend = willows.ico

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).inc

# override the standard include to add additional paths
TWIN_WININC = -I$(TWINDIR)/include -I$(TWINDIR)/win
TWIN_WINLIB = -L$(TWINDIR)/win

#####################################################################
# user makefile definitions for libraries are placed below
#####################################################################

libdllres.$(DLL_EXT): dllres.o dllres.res.o
	$(DLL_LD) $(DLL_LDOPT) $(TWIN_WINLIB) -o \
		libdllres.$(DLL_EXT) dllres.o dllres.res.o \
		$(DLL_LDLIB) 

dllres.res.o: dllres.res.c
	$(DLL_CC) $(DLL_CCOPT) -g $(CFLAGS) -c dllres.res.c

dllres.res.c: dllres.rc
	$(RC) -n table -EDLLres_LibMain $(TWIN_WININC) dllres.rc

libbitmaps.$(DLL_EXT): bitmaps.o bitmaps.res.o 
	$(DLL_LD) $(DLL_LDOPT) $(TWIN_WINLIB) -o \
		libbitmaps.$(DLL_EXT) bitmaps.o bitmaps.res.o $(DLL_LDLIB)

bitmaps.res.o: bitmaps.res.c
	$(DLL_CC) $(DLL_CCOPT) -g $(CFLAGS) -c bitmaps.res.c

bitmaps.res.c: bitmaps.rc willows.bmp
	$(RC) -n table -EBitmaps_LibMain -I$(TWINDIR)/include bitmaps.rc

clean-libs:
	rm -f libbitmaps.$(DLL_EXT) libdllres.$(DLL_EXT) \
	      bitmaps.res.c dllres.res.c bitmaps.res.o dllres.res.o \
	      bitmaps.exp   dllres.exp

clobber: clean

