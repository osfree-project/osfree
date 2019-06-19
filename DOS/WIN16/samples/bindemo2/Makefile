#####################################################################
#
#  makefile for bindemo2
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#####################################################################

include $(TWINDIR)/Makefile.config

project		= bindemo2
components	= bindemo2.o bindemo2.res.o libbinlib2.$(DLL_EXT)
libraries	= libbinlib2.$(DLL_EXT)
linklibs	= -lshell$(TWIN_APISIZE)
resource	= bindemo2
resource_depend	= willows.ico

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).inc

# override the standard twin_wininc to add additional directories...
TWIN_WININC = -I$(TWINDIR)/include -I$(TWINDIR)/win
TWIN_WINLIB = -L$(TWINDIR)/win

#####################################################################
# user makefile definitions for libraries are placed below
#####################################################################

libbinlib2.$(DLL_EXT): binlib2.o binlib2.res.o
	$(DLL_LD) $(DLL_LDOPT) $(TWIN_WINLIB) -o \
		libbinlib2.$(DLL_EXT) binlib2.o binlib2.res.o $(DLL_LDLIB)

binlib2.res.o: binlib2.res.c
	$(DLL_CC) $(DLL_CCOPT) -g $(CFLAGS) -c binlib2.res.c

binlib2.res.c: binlib2.rc
	$(RC) -n table -EBinlib_LibMain $(TWIN_WININC) binlib2.rc

clean-libs:
	rm -f libbinlib2.$(DLL_EXT) \
	      binlib2.res.c  binlib2.res.o  binlib2.exp  so_locations

clobber: clean

