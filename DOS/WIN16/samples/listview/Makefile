#####################################################################
#
#  @(#)makefile	1.3 	3/22/95 11:54:59
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#####################################################################

include $(TWINDIR)/Makefile.config

project		= listview
components	= listview.o listview.res.o
libraries	= 
linklibs	= -lshell$(TWIN_APISIZE) -lcommctrl32
resource	= listview
resource_depend = 

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).inc

# this adds twindir/twinsrc to get binary header files...
TWIN_WININC = -I$(TWINDIR)/include -I$(TWINDIR)/commctrl
TWIN_WINLIB = -L$(TWINDIR)/win

#####################################################################
# user makefile definitions for libraries are placed below
#####################################################################

clean-libs:

