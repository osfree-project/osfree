# $Id: makefile,v 1.4 2004/08/16 04:51:11 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR = $(ROOT)$(SEP)bin$(SEP)os2$(SEP)dll
PROJ = jpos2dll
DESC = CMD.EXE helper
OBJS = jpos2dll.$(O)
ADD_COPT = -i=..$(SEP)include
LINKOPT  = lib os2386.lib lib clib3r.lib
#LINKOPT = lib os2386#,clib3r#,os2$(SEP)os2 # cmd_shared.lib,all_shared.lib

!include $(%ROOT)/mk/all.mk

all: (PROJ).dll $(PROJ).sym

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk SYSTEM os2v2 dll initglobal
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk export    QueryExtLIBPATH.1, SetExtLIBPATH.2, SendKeys.3, QuitSendKeys.4
 @%append $^&.lnk export    StartHook.5, PauseKeys.6, KeystackHookProc.7
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)

# Clean target
.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
