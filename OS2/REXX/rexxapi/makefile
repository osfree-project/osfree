# $Id: makefile,v 1.3 2004/08/16 04:51:08 prokushev Exp $

PROJ = rexxapi
DESC = Classic REXX API
srcfiles     = $(p)rexxapi$(e)
ADD_COPT     = -i=. -bd -s
ADD_LINKOPT  = lib clib3r.lib,math387r.lib,emu387.lib,rexx.lib
DLL          = 1
DLLOPT       = initinstance terminstance
OPTIONS      = manyautodata, caseexact, heapsize=40960
STACKSIZE    = 0x20000
IMPORTS      = &
# IBM Classic REXX functions
    RexxDeregisterExit REGINA.RexxDeregisterExit, &
    RexxDeregisterFunction REGINA.RexxDeregisterFunction, &
    RexxDeregisterSubcom REGINA.RexxDeregisterSubcom, &
    RexxQueryExit REGINA.RexxQueryExit, &
    RexxQueryFunction REGINA.RexxQueryFunction, &
    RexxQuerySubcom REGINA.RexxQuerySubcom, &
    RexxRegisterExitDll REGINA.RexxRegisterExitDll, &
    RexxRegisterExitExe REGINA.RexxRegisterExitExe, &
    RexxRegisterFunctionDll REGINA.RexxRegisterFunctionDll, &
    RexxRegisterFunctionExe REGINA.RexxRegisterFunctionExe, &
    RexxRegisterSubcomDll REGINA.RexxRegisterSubcomDll, &
    RexxRegisterSubcomExe REGINA.RexxRegisterSubcomExe, &
    RexxAddMacro REGINA.RexxAddMacro, &
    RexxAddQueue REGINA.RexxAddQueue, &
    RexxClearMacroSpace REGINA.RexxClearMacroSpace, &
    RexxCreateQueue REGINA.RexxCreateQueue, &
    RexxDeleteQueue REGINA.RexxDeleteQueue, &
    RexxDropMacro REGINA.RexxDropMacro, &
    RexxLoadMacroSpace REGINA.RexxLoadMacroSpace, &
    RexxPullQueue REGINA.RexxPullQueue, &
    RexxQueryMacro REGINA.RexxQueryMacro, &
    RexxQueryQueue REGINA.RexxQueryQueue, &
    RexxReorderMacro REGINA.RexxReorderMacro, &
    RexxSaveMacroSpace REGINA.RexxSaveMacroSpace, &
# Regina REXX extensions
    RexxAllocateMemory REGINA.RexxAllocateMemory, &
    RexxFreeMemory REGINA.RexxFreeMemory, &
    ReginaVersion REGINA.ReginaVersion, &
    __regina_faked_main REGINA.__regina_faked_main, &
    RexxCallBack REGINA.RexxCallBack
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
    RexxAllocateMemory, &
    RexxFreeMemory, &
    ReginaVersion, &
    __regina_faked_main, &
    RexxCallBack, &
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

!include $(%ROOT)/mk/appsos2.mk

