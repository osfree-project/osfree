# $Id: makefile,v 1.4 2004/08/16 04:51:07 prokushev Exp $
#
#
#

# A linking error comes up without "32_BITS":
#    Error! E2064: cannot combine 32-bit segments (_DATA) with 16-bit segments (_BSS)
# The link works with 32 bit.
PROJ = rexx
DESC = Classic REXX
srcfiles     = $(p)rexx$(e) $(p)rexx16$(e) # $(p)debug$(e)
#srcfiles16   = $(p)rexx16$(e)
ADD_COPT     = -i=. -bd -s
ADD_LINKOPT  = segment   type DATA shared lib clib3r.lib,math387r.lib,emu387.lib,rexx.lib #,clibdls.lib
DLL          = 1
OPTIONS      = manyautodata, caseexact, mixed1632, heapsize=40960, quiet
IMPORTS      = RexxStart REGINA.RexxStart, &
               ReginaRexxStart REGINA.RexxStart, &
               RexxVariablePool REGINA.RexxVariablePool, &
               RexxSetHalt REGINA.RexxSetHalt
EXPORTS      = RexxStart.1, &
               RexxVariablePool.2, &
               RexxSetHalt.3, &
               RexxSetTrace.4, &
               RexxResetTrace.5, &
               RXTRACERESET.6 RESIDENT, &
               RXVAR.8 RESIDENT, &
               REXXSAA.9 RESIDENT, &
               RXHALTSET.10 RESIDENT,&
               RXTRACESET.11 RESIDENT, &
# This function not found in any of OS/2 Toolkits
               RXBREAKCLEANUP.7 RESIDENT
DLLOPT       = initinstance # initglobal
STACKSIZE    = 81920
DEST         = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
