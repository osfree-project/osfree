#
#
#
#

PROJ      = regina
TRGT     = $(PROJ).dll
DESC      = Regina interpreter DLL
DEST      = os2$(SEP)dll
STACKSIZE = 0x20000
OPTIONS   = heapsize=40960,quiet,caseexact,manyautodata
DLLOPT    = initinstance terminstance
DLL       = 1
ADD_COPT  = $(THREADING) $(HAVE_GCI) $(DYN_CFLAGS) -bd
ADD_LINKOPT  = segment   type DATA nonshared

srcfiles  = &
$(p)mygetopt$(e) &
$(p)client$(e) &
$(p)drexx$(e) &
$(p)rexxsaa$(e) &
$(p)funcs$(e) &
$(p)builtin$(e) &
$(p)error$(e) &
$(p)variable$(e) &
$(p)interprt$(e) &
$(p)debug$(e) &
$(p)dbgfuncs$(e) &
$(p)memory$(e) &
$(p)parsing$(e) &
$(p)files$(e) &
$(p)misc$(e) &
$(p)unxfuncs$(e) &
$(p)arxfuncs$(e) &
$(p)os2funcs$(e) &
$(p)cmsfuncs$(e) &
$(p)shell$(e) &
$(p)rexxext$(e) &
$(p)stack$(e) &
# $(p)os2que$(e) &
$(p)mktime$(e) &
$(p)tracing$(e) &
$(p)interp$(e) &
$(p)cmath$(e) &
$(p)convert$(e) &
$(p)strings$(e) &
$(p)library$(e) &
$(p)strmath$(e) &
$(p)signals$(e) &
$(p)macros$(e) &
$(p)envir$(e) &
$(p)$(THREADING_FILE)$(e) &
$(p)instore$(e) &
$(p)expr$(e) &
$(p)yaccsrc$(e) &
$(p)lexsrc$(e) &
$(p)wrappers$(e) &
$(p)options$(e) &
$(p)os_os2$(e) &
$(p)extstack$(e) &
$(p)rexxbif$(e) &
!ifneq GCI N
$(p)gci_call$(e) &
$(p)gci_convert$(e) &
$(p)gci_execute$(e) &
$(p)gci_helper$(e) &
$(p)gci_prepare$(e) &
$(p)gci_reginabridge$(e) &
$(p)gci_rxfuncdefine$(e) &
$(p)gci_tree$(e)
!endif

IMPORTS = &
 rexxCreateQueue REXXAPI.1, &
 rexxDeleteQueue REXXAPI.2, &
 rexxAddQueue    REXXAPI.3, &
 rexxPullQueue   REXXAPI.4, &
 rexxQueryQueue  REXXAPI.5

EXPORTS = &
 RexxAllocateMemory, &
 RexxDeregisterExit, &
 RexxDeregisterFunction, &
 RexxDeregisterSubcom,&
 RexxFreeMemory,&
 RexxQueryExit, &
 RexxQueryFunction, &
 RexxQuerySubcom, &
 RexxRegisterExitDll, &
 RexxRegisterExitExe, &
 RexxRegisterFunctionDll, &
 RexxRegisterFunctionExe, &
 RexxRegisterSubcomDll, &
 RexxRegisterSubcomExe, &
 RexxSetHalt, &
 RexxStart, &
 RexxVariablePool, &
 ReginaVersion, &
 __regina_faked_main, &
 RexxAddMacro, &
 RexxAddQueue, &
 RexxClearMacroSpace, &
 RexxCreateQueue, &
 RexxDeleteQueue, &
 RexxDropMacro, &
 RexxLoadMacroSpace, &
 RexxPullQueue, &
 RexxQueryMacro, &
 RexxQueryQueue, &
 RexxReorderMacro, &
 RexxSaveMacroSpace, &
 RexxCallBack, &
 ReginaCleanup

!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk
