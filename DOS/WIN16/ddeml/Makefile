#####################################################################
#
# @(#)Makefile	1.8
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#  Makefile for build DDEML dynamic link library
# 
#####################################################################

include $(TWINDIR)/Makefile.config

TWIN_AUXINC     = -I$(TWINDIR)/win -I.

project		= libddeml$(TWIN_APISIZE)
components	= DdeML.o DdeMLBin.o DdeML.res.o
libraries	= -lkeyboard$(TWIN_APISIZE)
resource	= DdeML

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).lib

install: $(TWINLIB)/$(project).$(DLL_EXT)

depend:
