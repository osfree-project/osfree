#####################################################################
#
#  makefile for prtdemo
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#####################################################################

include $(TWINDIR)/Makefile.config

project		= prtdemo 
components	= prtdemo.o prtdemo.res.o 
libraries	= 
linklibs	= -lshell$(TWIN_APISIZE) -lcommdlg$(TWIN_APISIZE)
resource	= prtdemo
resource_depend	= willows.ico

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).inc

#####################################################################
# user makefile definitions for libraries are placed below
#####################################################################

clean-libs:
