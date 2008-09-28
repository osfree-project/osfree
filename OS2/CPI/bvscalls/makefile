#
# A Makefile for bvscalls.dll
# (c) osFree project,
# author, date
#

PROJ        = bvscalls
DESC        = Base Video API
OBJS        = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT    = -i=..\include
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = BVSGETPTRDRAWNAME DOSCALLS.847, &
          BVSMAIN           DOSCALLS.845, &
          BVSREDRAWSIZE     DOSCALLS.846
EXPORTS = BVSGETPTRDRAWNAME .1 RESIDENT, &
          BVSMAIN           .3 RESIDENT, &
          BVSREDRAWSIZE     .4 RESIDENT
DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
