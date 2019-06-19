#
# A Makefile for bmscalls.dll
# (c) osFree project,
# author, date
#

PROJ        = bmscalls
TRGT        = $(PROJ).dll
DESC        = Base Mouse API
OBJS        = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT    = -i=..\include
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = BMSMAIN DOSCALLS.852
EXPORTS = BMSMAIN.1 RESIDENT
DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
