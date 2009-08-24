#####################################################################
#
#  makefile for bindemo
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#####################################################################

include $(TWINDIR)/Makefile.config

project		= bindemo
components	= bindemo.o bindemo.res.o
linklibs	= -lshell$(TWIN_APISIZE)
resource	= bindemo
resource_depend	= willows.ico

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).inc

# this adds twindir/twinsrc to get binary header files...
TWIN_WININC = -I$(TWINDIR)/include -I$(TWINDIR)/win
TWIN_WINLIB = -L$(TWINDIR)/win

#####################################################################
# user makefile definitions for libraries are placed below
#####################################################################

clean-libs:
