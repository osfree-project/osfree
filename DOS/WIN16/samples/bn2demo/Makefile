#####################################################################
#
#  makefile for bn2demo
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#####################################################################

include $(TWINDIR)/Makefile.config

project		= bindemo
components	= bindemo.o bindemo.res.o binlib.$(DLL_EXT)
libraries	= binlib.$(DLL_EXT)
linklibs	= -lshell$(TWIN_APISIZE)
resource	= bindemo
resource_depend	= willows.ico

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).inc

# override standard twin_wininc to add additional directories
TWIN_WININC = -I$(TWINDIR)/include -I$(TWINDIR)/win
TWIN_WINLIB = -L$(TWINDIR)/win

#####################################################################
# user makefile definitions for libraries are placed below
#####################################################################

binlib.$(DLL_EXT): binlib.o binlib.res.o
	$(DLL_LD) $(DLL_LDOPT) $(TWIN_WINLIB) -o \
		binlib.$(DLL_EXT) binlib.o binlib.res.o $(DLL_LDLIB)

binlib.res.o: binlib.res.c
	$(DLL_CC) $(DLL_CCOPT) -g $(CFLAGS) -c binlib.res.c

binlib.res.c: binlib.rc
	$(RC) -n table $(TWIN_INCDIR) binlib.rc

binlib.rc:
	touch binlib.rc

clean-libs:
	rm -f binlib.$(DLL_EXT) binlib.res.c binlib.res.o binlib.exp  

