# $Id: makefile,v 1.4 2004/08/16 04:51:07 prokushev Exp $
#
#
#

PROJ = rexxinit
DESC = REXX initializer DLL
srcfiles     = $(p)rexxinit$(e)
ADD_COPT     = -i=. -sg -3s
ADD_LINKOPT  = segment   type DATA shared lib clib3r.lib,math387r.lib,emu387.lib,rexx.lib
DLL          = 1
DLLOPT       = initinstance
OPTIONS      = manyautodata, caseexact, heapsize=40960
STACKSIZE    = 81920
EXPORTS      = RxApiInit.1, &
               dllinit.2

DEST         = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
