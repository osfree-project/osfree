# $Id: makefile,v 1.2 2004/08/16 04:51:17 prokushev Exp $

ROOT=..\..
!include $(ROOT)\makefile.inc

# Add other targets here upon their maturity
DLLS=shared msg nls sesmgr kbdcalls moucalls quecalls viocalls #doscalls

all
 $(SAY) Building Control Programming Interface
 @for %%i in ($(DLLS)) do @cd %%i&$(MAKE) $(MAKEFLAGS)&cd ..

clean
 $(SAY) Cleaning Control Programming Interface
 @for %%i in ($(DLLS)) do @cd %%i&$(MAKE) $(MAKEFLAGS) clean&cd ..
 @cd shared
 -@del *.obj $(LOG)
 -@del *.err $(LOG)
 @cd ..