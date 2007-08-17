# $Id: makefile,v 1.4 2004/08/16 04:51:11 prokushev Exp $

ROOT=..\..\..\..

DIR = $(ROOT)\bin\os2\dll
PROJ = jpos2dll
DESC = CMD.EXE helper
OBJS = jpos2dll.obj
ADD_COPT = -i=..\include
#ADD_LIB = cmd_shared.lib,all_shared.lib

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).dll $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk file $(OBJS)
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @%append $^&.lnk export    QueryExtLIBPATH.1, SetExtLIBPATH.2, SendKeys.3, QuitSendKeys.4
 @%append $^&.lnk export    StartHook.5, PauseKeys.6, KeystackHookProc.7
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)

# Clean all

clean: .symbolic
 -@del /q *.obj $(LOG)
 -@del /q *.sbr $(LOG)
 -@del /q *.map $(LOG)
 -@del /q *.err $(LOG)
 -@del /q *.wmp $(LOG)
 -@del /q *.lnk $(LOG)
