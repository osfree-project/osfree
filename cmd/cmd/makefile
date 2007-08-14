# $Id: makefile,v 1.3 2004/08/16 04:51:10 prokushev Exp $

ROOT=..\..\..
!include $(ROOT)\makefile.inc

# Add other targets here upon their maturity
PARTS=shared cmd help keystack shralias option2 jpos2dll #vdm2os2

all
 @for %%i in ($(PARTS)) do @cd %%i&$(MAKE) $(MAKEFLAGS)&cd ..

clean
 @for %%i in ($(PARTS)) do @cd %%i&$(MAKE) $(MAKEFLAGS) clean&cd ..

