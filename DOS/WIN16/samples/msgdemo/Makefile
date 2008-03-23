#####################################################################
#  
#  makefile for msgdemo
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#####################################################################

include $(TWINDIR)/Makefile.config

project		= msgdemo
components	= msgdemo.o msgdemo.res.o msgcode.o
libraries	= 
linklibs	= -lshell$(TWIN_APISIZE)
resource	= msgdemo
resource_depend	= willows.ico

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).inc

#####################################################################
# user makefile definitions for libraries are placed below
#####################################################################

clean-libs:
