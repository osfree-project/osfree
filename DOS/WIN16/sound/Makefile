#####################################################################
#
# @(#)Makefile	1.8
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#  Makefile for build SOUND dynamic link library
# 
#####################################################################

include $(TWINDIR)/Makefile.config

TWIN_AUXINC     = -I$(TWINDIR)/win -I.

project		= libsound$(TWIN_APISIZE)
components	= Sound.o SoundBin.o Sound.res.o
libraries	= 
resource	= Sound

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).lib

install: $(TWINLIB)/$(project).$(DLL_EXT)

depend:
