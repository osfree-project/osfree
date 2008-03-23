#####################################################################
#
# @(#)Makefile	1.8
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#  Makefile for build KEYBOARD dynamic link library
# 
#####################################################################

include $(TWINDIR)/Makefile.config

TWIN_AUXINC     = -I$(TWINDIR)/win -I.

project		= libkeyboard$(TWIN_APISIZE)
components	= Keyboard.o SysKeyboard.o KeyboardIF.o KeyboardBin.o Keyboard.res.o
libraries	= 
resource	= Keyboard

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).lib

install: $(TWINLIB)/$(project).$(DLL_EXT)

depend:
