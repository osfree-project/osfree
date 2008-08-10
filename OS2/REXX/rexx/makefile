# $Id: makefile,v 1.4 2004/08/16 04:51:07 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS = 1

CLEANMASK = *.obj *.map *.err *.wmp *.lnk *.sym *.lib
DIR = $(FILESDIR)$(SEP)os2$(SEP)dll
PROJ = rexx
DESC = Classic REXX
OBJS = rexx.$(O) rexx16.$(O) debug.$(O)
ADD_COPT = -i=. -bd -s
LINKLIB = clib3r.lib,math387r.lib,emu387.lib,rexx.lib #,clibdls.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll $(PROJ).sym

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initinstance terminstance
 @%append $^&.lnk option    ManyAutoData
 @%append $^&.lnk option    caseexact
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk option stack=0x20000
 @%append $^&.lnk option heapsize=40960
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk OPTION MIXED1632
 @%append $^&.lnk OPTION QUIET
 @%append $^&.lnk LIBPATH $(%WATCOM)$(SEP)lib386$(PS)$(%WATCOM)$(SEP)lib386$(SEP)os2
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @%append $^&.lnk import
 @%append $^&.lnk   RexxStart REGINA.RexxStart,
 @%append $^&.lnk   ReginaRexxStart REGINA.RexxStart,
 @%append $^&.lnk   RexxVariablePool REGINA.RexxVariablePool,
 @%append $^&.lnk   RexxSetHalt REGINA.RexxSetHalt
 @%append $^&.lnk export
 @%append $^&.lnk   RexxStart.1,
 @%append $^&.lnk   RexxVariablePool.2,
 @%append $^&.lnk   RexxSetHalt.3,
 @%append $^&.lnk   RexxSetTrace.4,
 @%append $^&.lnk   RexxResetTrace.5,
 @%append $^&.lnk   RXTRACERESET.6 RESIDENT,
 @%append $^&.lnk   RXVAR.8 RESIDENT,
 @%append $^&.lnk   REXXSAA.9 RESIDENT,
 @%append $^&.lnk   RXHALTSET.10 RESIDENT,
 @%append $^&.lnk   RXTRACESET.11 RESIDENT,
 # This function not found in any of OS/2 Toolkits
 @%append $^&.lnk   RXBREAKCLEANUP.7 RESIDENT
 $(ADDFILES_CMD)
 $(SAY) Linking $^@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 @$(MAKE) $(MAKEOPT) install

#lib: .SYMBOLIC
# @$(LIB) $(LIBOPT) -n clibdls.lib

#rexx16.obj: rexx16.c
# @$(MAKE) $(MAKEOPT) -f makefl16 all

# Clean all

clean: .symbolic
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).dll $(FILESDIR)$(SEP)os2$(SEP)dll
