# $Id: makefile,v 1.5 2004/08/21 14:48:44 prokushev Exp $

ROOT=..\..\..
!include $(ROOT)\makefile.inc

# Add other targets here upon their maturity
LIBS=zlib

all
 $(SAY) Building libraries.
 @for %%i in ($(LIBS)) do @cd %%i&echo Building %%i tree&$(MAKE) $(MAKEFLAGS)&cd ..

clean
 $(SAY) Cleaning libraries
 @for %%i in ($(LIBS)) do @cd %%i&$(MAKE) $(MAKEFLAGS) clean&cd ..
