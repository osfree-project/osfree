# $Id: makefile,v 1.5 2004/08/16 04:51:07 prokushev Exp $

ROOT=..\..
!include $(ROOT)\makefile.inc

# Add other targets here upon their maturity
TARGETS=interp rexx rexxapi # libs

all
 $(SAY) Building REXX
 @for %%i in ($(TARGETS)) do @cd %%i&$(MAKE) $(MAKEFLAGS)&cd ..

clean
 $(SAY) Cleaning REXX
 @for %%i in ($(TARGETS)) do @cd %%i&$(MAKE) $(MAKEFLAGS) clean&cd ..
