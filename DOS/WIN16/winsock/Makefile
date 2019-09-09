#####################################################################
#
# @(#)Makefile	1.8
#
#  Copyright (c) 1995, Willows Software Inc.  All rights reserved.
#
#  Makefile for build winsock dynamic link library
# 
#####################################################################

include $(TWINDIR)/Makefile.config

TWIN_AUXINC     = -I$(TWINDIR)/win -I.

project		= libwinsock$(TWIN_APISIZE)
components	= WinSock.o WinSockIF.o WinSock.res.o WinSockBin.o
libraries	= 
resource	= WinSock
#resource_flags	= -EWinSock_LibMain

include $(TWINDIR)/makeinclude/makefile$(TWIN_APISIZE).lib

install: $(TWINLIB)/$(project).$(DLL_EXT)

depend:
