#####################################################################
#
#  @(#)makefile	1.3 	3/22/95 11:54:59
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#####################################################################

include $(TWINDIR)/Makefile.config

project		= windemo
components	= windemo.o windemo.res.o
libraries	= 
linklibs	= -lshell$(TWIN_APISIZE)
resource	= windemo
resource_depend = willows.ico

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).inc

#####################################################################
# user makefile definitions for libraries are placed below
#####################################################################

clean-libs:
