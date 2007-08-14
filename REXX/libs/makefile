# $Id: makefile,v 1.1.1.1 2003/10/04 08:34:54 prokushev Exp $

ROOT=..\..\..
!include $(ROOT)\makefile.inc

# Add other targets here upon their maturity
TARGETS=rexxutil

all
 for %%i in ($(TARGETS)) do cd %%i&$(MAKE) $(MAKEFLAGS)&cd ..

