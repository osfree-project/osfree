
!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
# A linking error comes up without "32_BITS":
#    Error! E2064: cannot combine 32-bit segments (_DATA) with 16-bit segments (_BSS)
# The link works with 32 bit.
DIR = $(FILESDIR)$(SEP)os2$(SEP)dll
PROJ = jpos2dll
DESC = CMD.EXE helper
OBJS = jpos2dll.$(O)
#$(O)  jpos2dll.c
ADD_COPT = -i=..$(SEP)include -bd
#ADD_LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll #$(PROJ).sym

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk SYSTEM os2v2 dll initglobal
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk option    caseexact
 @%append $^&.lnk option    mixed1632
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk export    QueryExtLIBPATH.1, SetExtLIBPATH.2, SendKeys.3, QuitSendKeys.4
 @%append $^&.lnk export    StartHook.5, PauseKeys.6, KeystackHookProc.7
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 @$(MAKE) $(MAKEOPT) install

# Clean target
.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).dll $(DIR)
