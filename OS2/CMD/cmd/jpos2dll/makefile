#
#
#

# A linking error comes up without "32_BITS":
#    Error! E2064: cannot combine 32-bit segments (_DATA) with 16-bit segments (_BSS)
# The link works with 32 bit.
PROJ = jpos2dll
DESC = CMD.EXE helper
srcfiles = $(p)jpos2dll$(e)
ADD_COPT = -i=$(MYDIR)..$(SEP)include -bd
ADD_LINKOPT  = segment   type DATA shared # lib os2386.lib lib clib3r.lib
DLL      = 1
OPTIONS  = manyautodata, caseexact, mixed1632
DLLOPT   = initglobal
EXPORTS  = SendKeys.3, QuitSendKeys.4, &
           StartHook.5, PauseKeys.6, KeystackHookProc.7
DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
