#
# A Makefile for bdcalls.dll
# (c) osFree project,
# author, date
#

PROJ     = bdcalls
TRGT     = $(PROJ).dll
DESC     = osFree BiDirectional Language APIs
#OBJS    = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
32_BITS  = 0
srcfiles = $(p)bdcalls$(e)
ADD_COPT = -bd -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until UniAPI modules will be ready
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL      = 1
DLLOPT   = initinstance terminstance
OPTIONS  = manyautodata

EXPORTS  = &
   NLSCONVERTBIDINUMERICS.1=NLS16CONVERTBIDINUMERICS, &
   NLSCONVERTBIDISTRING.2=NLS16CONVERTBIDISTRING, &
   NLSINTSETBIDIATT.3=NLS16INTSETBIDIATT, &
   NLSINTQUERYBIDIATT.4=NLS16INTQUERYBIDIATT, &
   NLSSETBIDIATT.5=NLS16SETBIDIATT, &
   NLSQUERYBIDIATT.6=NLS16QUERYBIDIATT, &
   NLSINVERSESTRING.7=NLS16INVERSESTRING, &
   NLSSETBIDIPRINT.8=NLS16SETBIDIPRINT, &
   NLSEDITSHAPE.9=NLS16EDITSHAPE, &
   NLSSHAPEBIDISTRING.10=NLS16SHAPEBIDISTRING, &
   NLPOPUP.11=NL16POPUP

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
