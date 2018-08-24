# $Id: makefile,v 1.3 2004/08/16 04:51:08 prokushev Exp $

PROJ = rexxapi
TRGT = $(PROJ).dll
TARGETS = $(PATH)wrappers.h $(PATH)$(TRGT)
DESC = REXX API forwarder for Regina
srcfiles     = $(p)rexxapi$(e) $(p)rexxapi16$(e) $(p)stk$(e)
# number of 16-bit subcommand wrappers (see rexxapi16.c)
NUMWRAP = 1024
ADD_COPT     = -dSUBCOM_COUNT=$(NUMWRAP) -i=. -i=$(PATH) -sg -of+ -bd -3s
ADD_LINKOPT  = segment   type DATA nonshared
DLL          = 1
NOLIBS       = 1
DLLOPT       = initinstance terminstance
OPTIONS      = manyautodata, caseexact, heapsize=40960, start=dll_initterm
STACKSIZE    = 81920
EXPORTS      = &
    RexxCreateQueue.1, &
    RexxDeleteQueue.2, &
    RexxAddQueue.3, &
    RexxPullQueue.4, &
    RexxQueryQueue.5, &
    RexxRegisterSubcomDll.6, &
    RexxRegisterSubcomExe.7, &
    RexxQuerySubcom.8, &
    RexxDeregisterSubcom.9, &
    RexxRegisterExitDll.11, &
    RexxRegisterExitExe.12, &
    RexxDeregisterExit.13, &
    RexxQueryExit.14, &
    RexxRegisterFunctionDll.17, &
    RexxRegisterFunctionExe.18, &
    RexxDeregisterFunction.19, &
    RexxQueryFunction.20, &
    RexxAddMacro.22, &
    RexxDropMacro.23, &
    RexxClearMacroSpace.24, &
    RexxLoadMacroSpace.25, &
    RexxQueryMacro.26, &
    RexxReorderMacro.27, &
    RexxSaveMacroSpace.28, &
# Undocumented IBM Classic REXX functions
    RexxLoadSubcom.10, &
    RexxCallExit.15, &
    RexxCallSubcom.16, &
    RexxCallFunction.21, &
    RexxExecuteMacroFunction.29, &
# Regina extensions (export without ordinals)
#    RexxAllocateMemory, &
#    RexxFreeMemory, &
#    ReginaVersion, &
#    __regina_faked_main, &
#    RexxCallBack, &
# 16bit versions of calls
    RXMACROLOAD.30 RESIDENT, &
    RXMACROSAVE.31 RESIDENT, &
    RXSUBCOMLOAD.32 RESIDENT, &
    RXMACRODROP.33 RESIDENT, &
    RXEXITQUERY.34 RESIDENT, &
    RXMACROERASE.35 RESIDENT, &
    RXSUBCOMDROP.36 RESIDENT, &
    RXMACROQUERY.37 RESIDENT, &
    RXMACROCHANGE.38 RESIDENT, &
    RXSUBCOMQUERY.39 RESIDENT, &
    RXFUNCTIONCALL.40 RESIDENT, &
    RXEXITREGISTER.41 RESIDENT, &
    RXMACROREORDER.42 RESIDENT, &
    RXSUBCOMEXECUTE.43 RESIDENT, &
    RXSUBCOMREGISTER.44 RESIDENT, &
    RXFUNCTIONQUERY.45 RESIDENT, &
    RXFUNCTIONREGISTER.46 RESIDENT, &
    RXEXITDROP.47 RESIDENT, &
    RXFUNCTIONDEREGISTER.48 RESIDENT

DEST         = os2$(SEP)dll
UNI2H        = 1

.h: $(PATH)

!include $(%ROOT)/mk/appsos2_cmd.mk

$(PATH)rexxapi16.obj: $(PATH)wrappers.h

$(PATH)wrappers.h:
 @$(REXX) genwrap.cmd $(NUMWRAP) >$^@
