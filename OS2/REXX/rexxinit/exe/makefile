# $Id: makefile,v 1.4 2004/08/16 04:51:07 prokushev Exp $
#
#
#

PROJ = rexxinit
DESC = REXX DLL loader
srcfiles     = $(p)rexxinit$(e)
ADD_COPT     = -i=. -sg -3s # -d3
ADD_LINKOPT  = segment type DATA shared lib os2386.lib,clib3s.lib,math387r.lib,emu387.lib # debug all

DEST         = os2

!include $(%ROOT)/mk/appsos2.mk
