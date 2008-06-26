# $Id: makefile,v 1.3 2004/08/16 04:51:08 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS = 1

CLEANMASK = *.obj *.map *.err *.wmp *.lnk *.sym *.lib
DIR = $(%ROOT)\files\os2\dll
PROJ = rexxapi
DESC = Classic REXX API
OBJS = rexxapi.obj
ADD_COPT = -i=. -bd -s
LINKLIB = clib3r.lib,math387r.lib,emu387.lib,rexx.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll $(PROJ).sym

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initinstance terminstance
 @%append $^&.lnk option    ManyAutoData
 @%append $^&.lnk option    caseexact
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk LIBPATH $(%WATCOM)$(SEP)lib386$(PS)$(%WATCOM)$(SEP)lib386$(SEP)os2
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @%append $^&.lnk import
# IBM Classic REXX functions
 @%append $^&.lnk   RexxDeregisterExit REGINA.RexxDeregisterExit,
 @%append $^&.lnk   RexxDeregisterFunction REGINA.RexxDeregisterFunction,
 @%append $^&.lnk   RexxDeregisterSubcom REGINA.RexxDeregisterSubcom,
 @%append $^&.lnk   RexxQueryExit REGINA.RexxQueryExit,
 @%append $^&.lnk   RexxQueryFunction REGINA.RexxQueryFunction,
 @%append $^&.lnk   RexxQuerySubcom REGINA.RexxQuerySubcom,
 @%append $^&.lnk   RexxRegisterExitDll REGINA.RexxRegisterExitDll,
 @%append $^&.lnk   RexxRegisterExitExe REGINA.RexxRegisterExitExe,
 @%append $^&.lnk   RexxRegisterFunctionDll REGINA.RexxRegisterFunctionDll,
 @%append $^&.lnk   RexxRegisterFunctionExe REGINA.RexxRegisterFunctionExe,
 @%append $^&.lnk   RexxRegisterSubcomDll REGINA.RexxRegisterSubcomDll,
 @%append $^&.lnk   RexxRegisterSubcomExe REGINA.RexxRegisterSubcomExe,
 @%append $^&.lnk   RexxAddMacro REGINA.RexxAddMacro,
 @%append $^&.lnk   RexxAddQueue REGINA.RexxAddQueue,
 @%append $^&.lnk   RexxClearMacroSpace REGINA.RexxClearMacroSpace,
 @%append $^&.lnk   RexxCreateQueue REGINA.RexxCreateQueue,
 @%append $^&.lnk   RexxDeleteQueue REGINA.RexxDeleteQueue,
 @%append $^&.lnk   RexxDropMacro REGINA.RexxDropMacro,
 @%append $^&.lnk   RexxLoadMacroSpace REGINA.RexxLoadMacroSpace,
 @%append $^&.lnk   RexxPullQueue REGINA.RexxPullQueue,
 @%append $^&.lnk   RexxQueryMacro REGINA.RexxQueryMacro,
 @%append $^&.lnk   RexxQueryQueue REGINA.RexxQueryQueue,
 @%append $^&.lnk   RexxReorderMacro REGINA.RexxReorderMacro,
 @%append $^&.lnk   RexxSaveMacroSpace REGINA.RexxSaveMacroSpace,
# Regina REXX extensions
 @%append $^&.lnk   RexxAllocateMemory REGINA.RexxAllocateMemory,
 @%append $^&.lnk   RexxFreeMemory REGINA.RexxFreeMemory,
 @%append $^&.lnk   ReginaVersion REGINA.ReginaVersion,
 @%append $^&.lnk   __regina_faked_main REGINA.__regina_faked_main,
 @%append $^&.lnk   RexxCallBack REGINA.RexxCallBack
 @%append $^&.lnk export
 @%append $^&.lnk   RexxCreateQueue.1,
 @%append $^&.lnk   RexxDeleteQueue.2,
 @%append $^&.lnk   RexxAddQueue.3,
 @%append $^&.lnk   RexxPullQueue.4,
 @%append $^&.lnk   RexxQueryQueue.5,
 @%append $^&.lnk   RexxRegisterSubcomDll.6,
 @%append $^&.lnk   RexxRegisterSubcomExe.7,
 @%append $^&.lnk   RexxQuerySubcom.8,
 @%append $^&.lnk   RexxDeregisterSubcom.9,
 @%append $^&.lnk   RexxRegisterExitDll.11,
 @%append $^&.lnk   RexxRegisterExitExe.12,
 @%append $^&.lnk   RexxDeregisterExit.13,
 @%append $^&.lnk   RexxQueryExit.14,
 @%append $^&.lnk   RexxRegisterFunctionDll.17,
 @%append $^&.lnk   RexxRegisterFunctionExe.18,
 @%append $^&.lnk   RexxDeregisterFunction.19,
 @%append $^&.lnk   RexxQueryFunction.20,
 @%append $^&.lnk   RexxAddMacro.22,
 @%append $^&.lnk   RexxDropMacro.23,
 @%append $^&.lnk   RexxClearMacroSpace.24,
 @%append $^&.lnk   RexxLoadMacroSpace.25,
 @%append $^&.lnk   RexxQueryMacro.26,
 @%append $^&.lnk   RexxReorderMacro.27,
 @%append $^&.lnk   RexxSaveMacroSpace.28,
# Undocumented IBM Classic REXX functions
# %append $^&.lnk   RexxLoadSubcom.10,
# %append $^&.lnk   RexxCallExit.15,
# %append $^&.lnk   RexxCallSubcom.16,
# %append $^&.lnk   RexxCallFunction.21,
# %append $^&.lnk   RexxExecuteMacroFunction.29,
# Regina extensions (export without ordinals)
 @%append $^&.lnk   RexxAllocateMemory,
 @%append $^&.lnk   RexxFreeMemory,
 @%append $^&.lnk   ReginaVersion,
 @%append $^&.lnk   __regina_faked_main,
 @%append $^&.lnk   RexxCallBack,
# 16bit versions of calls
 @%append $^&.lnk   RXMACROLOAD.30 RESIDENT,
 @%append $^&.lnk   RXMACROSAVE.31 RESIDENT,
 @%append $^&.lnk   RXSUBCOMLOAD.32 RESIDENT,
 @%append $^&.lnk   RXMACRODROP.33 RESIDENT,
 @%append $^&.lnk   RXEXITQUERY.34 RESIDENT,
 @%append $^&.lnk   RXMACROERASE.35 RESIDENT,
 @%append $^&.lnk   RXSUBCOMDROP.36 RESIDENT,
 @%append $^&.lnk   RXMACROQUERY.37 RESIDENT,
 @%append $^&.lnk   RXMACROCHANGE.38 RESIDENT,
 @%append $^&.lnk   RXSUBCOMQUERY.39 RESIDENT,
 @%append $^&.lnk   RXFUNCTIONCALL.40 RESIDENT,
 @%append $^&.lnk   RXEXITREGISTER.41 RESIDENT,
 @%append $^&.lnk   RXMACROREORDER.42 RESIDENT,
 @%append $^&.lnk   RXSUBCOMEXECUTE.43 RESIDENT,
 @%append $^&.lnk   RXSUBCOMREGISTER.44 RESIDENT,
 @%append $^&.lnk   RXFUNCTIONQUERY.45 RESIDENT,
 @%append $^&.lnk   RXFUNCTIONREGISTER.46 RESIDENT,
 @%append $^&.lnk   RXEXITDROP.47 RESIDENT,
 @%append $^&.lnk   RXFUNCTIONDEREGISTER.48 RESIDENT
 $(ADDFILES_CMD)
 $(SAY) Linking $^@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 @$(MAKE) $(MAKEOPT) install

$(DIR)$(SEP)$(PROJ).sym: $(PROJ).sym
 $(CP) *.sym $(DIR)

$(PROJ).sym: $(PROJ).map

$(PROJ).map: $(PROJ).wmp

# Clean all
clean: .symbolic
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).dll $(FILESDIR)$(SEP)os2$(SEP)dll
