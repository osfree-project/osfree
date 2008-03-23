#####################################################################
#
# @(#)Makefile	1.8
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#  Makefile for build shell dynamic link library
# 
#####################################################################

include $(TWINDIR)/Makefile.config

TWIN_AUXINC     = -I$(TWINDIR)/win

project		= libshell$(TWIN_APISIZE)
components	= Shell.o ShellIF.o Registry.o DragDrop.o About.o Shell.res.o
libraries	= 
resource	= Shell
resource_flags	= -EShell_LibMain

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).lib

install: $(TWINLIB)/$(project).$(DLL_EXT)

depend:
	@-makedepend -f Makedepend.$(TARGET) $(DEPEND) -I. $(CFLAGS) Shell.c Registry.c DragDrop.c About.c 2>/dev/null
