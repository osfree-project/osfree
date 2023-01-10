# $Id: makefile,v 1.4 2004/08/16 04:51:07 prokushev Exp $
#
#
#

PROJ = rexxinit
TRGT = $(PROJ).exe
DESC = REXX DLL loader
srcfiles     = $(p)rexxinit$(e)
ADD_COPT     = -i=. -sg -3r # -d3
ADD_LINKOPT  = segment type DATA shared # lib os2386.lib,clib3r.lib,math387r.lib,emu387.lib # debug all
UNI2H        = 1
DEST         = os2

!include $(%ROOT)tools/mk/appsos2_cmd.mk
