#*************************************************************************
# @(#)Makefile	1.18 
#
#  Makefile for Commdlg
#
#  Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
#
#*************************************************************************/

include $(TWINDIR)/Makefile.config
TWIN_AUXINC     = -I$(TWINDIR)/win -I$(TWINDIR)

project		= libcommdlg$(TWIN_APISIZE)
components	= ChooseColor.o ChooseFont.o Commdlg.o  \
		FindReplDlg.o GetOpenFileName.o PrintDlg.o \
		CommdlgIF.o CommdlgIT.o Commdlg.res.o

libraries	= 
resource	= Commdlg
resource_flags	= -ECommdlg_LibMain
localclean	= 

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).lib

install: $(TWINLIB)/$(project)$(DLL_EXT) $(TWINLIB)/$(project).$(DLL_EXT)

install32:
	$(MAKE) install

all32:
	$(MAKE) all $(PASSFLAGS) TWIN_APISIZE=32

$(TWINLIB)/$(project).$(DLL_EXT): $(project)$(DLL_EXT)
	rm -f $(TWINLIB)/$(project).$(DLL_EXT)
	cp $(project).$(DLL_EXT) $(TWINLIB)/$(project).$(DLL_EXT)
	chmod 755 $(TWINLIB)/$(project).$(DLL_EXT)

depend:
