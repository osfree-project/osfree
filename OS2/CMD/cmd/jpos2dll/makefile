#
#
#

# A linking error comes up without "32_BITS":
#    Error! E2064: cannot combine 32-bit segments (_DATA) with 16-bit segments (_BSS)
# The link works with 32 bit.
PROJ = jpos2dll
TRGT = $(PROJ).dll
DESC = CMD.EXE helper
srcfiles = $(p)jpos2dll$(e)
ADD_LINKOPT  = segment   type DATA shared # lib os2386.lib lib clib3r.lib
DLL      = 1
OPTIONS  = manyautodata, caseexact #, mixed1632
DLLOPT   = initglobal
EXPORTS  = SendKeys.3, QuitSendKeys.4, &
           StartHook.5, PauseKeys.6, KeystackHookProc.7
#UNI2H = 1
DEST     = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
!include $(MYDIR)..$(SEP)port$(SEP)4os2.mk

ADD_COPT = -i=$(PORT_BASE)h -i=$(MYDIR)..$(SEP)..$(SEP)include -i=$(BLD)include &
     -i=$(%ROOT)include$(SEP)os3 -i=$(BLD)include -s -bd

.c: $(PORT_BASE)c

.h: $(PORT_BASE)h
