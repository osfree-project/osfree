# $Id: makefile,v 1.4 2004/08/16 04:51:07 prokushev Exp $
#
#
#

# A linking error comes up without "32_BITS":
#    Error! E2064: cannot combine 32-bit segments (_DATA) with 16-bit segments (_BSS)
# The link works with 32 bit.
PROJ = rexx
DESC = Classic REXX
srcfiles     = $(p)rexx$(e) $(p)rexx16$(e) $(p)stk$(e)
ADD_COPT     = -i=. -sg -of+
ADD_LINKOPT  = segment   type DATA nonshared lib clib3r.lib,math387r.lib,emu387.lib
DLL          = 1
DLLOPT       = initinstance
OPTIONS      = manyautodata, caseexact, heapsize=40960, start=dll_initterm
EXPORTS      = RexxStart.1, &
               RexxVariablePool.2, &
               RexxSetHalt.3, &
               RexxSetTrace.4, &
               RexxResetTrace.5, &
               RXTRACERESET.6 RESIDENT, &
               RXVAR.8 RESIDENT, &
               REXXSAA.9 RESIDENT, &
               RXHALTSET.10 RESIDENT, &
               RXTRACESET.11 RESIDENT, &
# This function not found in any of OS/2 Toolkits
               RXBREAKCLEANUP.7 RESIDENT
DEST         = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
