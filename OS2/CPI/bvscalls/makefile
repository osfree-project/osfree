#
# A Makefile for bvscalls.dll
# (c) osFree project,
# author, date
#

PROJ        = bvscalls
TRGT        = $(PROJ).dll
DESC        = Base Video Subsystem API
srcfiles = $(p)dllstart$(e) &
           $(p)bvs32main$(e)
ADD_COPT    = -I=$(MYDIR)..$(SEP)include
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = BVSGETPTRDRAWNAME DOSCALLS.847, &
          BVSMAIN           DOSCALLS.845, &
          BVSREDRAWSIZE     DOSCALLS.846, &
          DosLogWrite       DOSCALLS.1112
		  
EXPORTS = BVSGETPTRDRAWNAME .1 RESIDENT, &
          BVSMAIN           .3 RESIDENT, &
          BVSREDRAWSIZE     .4 RESIDENT, &
          BVS32Main         .103
DEST    = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
