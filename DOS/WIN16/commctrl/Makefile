#####################################################################
#  @(#)Makefile	1.26 
#
#  Makefile for building common controls library
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#####################################################################

include $(TWINDIR)/Makefile.config
TWIN_AUXINC = -I$(TWINDIR)/win -I$(TWINDIR)

project		= libcommctrl$(TWIN_APISIZE)
components	= WCommctl.o WDragLbx.o WHdr.o WImgList.o \
		  WLVHelpr.o WLView.o WLists.o WProgBar.o \
		  WPropsht.o WStatusb.o WTVLBox.o WTView.o \
		  WTab.o WTlbCust.o WToolbar.o WTooltip.o  \
	          WTrackbr.o WUpdown.o WCommCtrl.res.o WLVLBox.o
libraries	= -lm
resource	= WCommCtrl
resource_flags	= -E$(project)_LibMain

include $(TWINDIR)/makeinclude/makefile32.lib

install32:
	$(MAKE) install

install: $(TWINLIB)/$(project).$(DLL_EXT)

all32:
	$(MAKE) all $(PASSFLAGS) 

$(TWINLIB)/$(project).$(DLL_EXT): $(project).$(DLL_EXT)
	rm -f $(TWINLIB)/$(project).$(DLL_EXT)
	cp $(project).$(DLL_EXT) $(TWINLIB)/$(project).$(DLL_EXT)
	chmod 755 $(TWINLIB)/$(project)$(DLL_EXT)

depend:
