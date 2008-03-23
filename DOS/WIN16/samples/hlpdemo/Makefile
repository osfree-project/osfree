#####################################################################
#
#  makefile for hlpdemo
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#####################################################################

include $(TWINDIR)/Makefile.config

project		= hlpdemo
components	= hlpdemo.o hlpdemo.res.o
libraries	= 
linklibs	= -lshell$(TWIN_APISIZE)
resource	= hlpdemo
resource_depend = willows.ico hlpdemo.hlp

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).inc

#####################################################################
# user makefile definitions for libraries are placed below
#####################################################################

clean-libs:
