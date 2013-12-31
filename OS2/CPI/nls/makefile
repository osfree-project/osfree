#
# A Makefile for nls.dll
# (c) osFree project,
# author, date
#

PROJ     = nls
DESC     = National Language Support API
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..$(SEP)include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = &
# 16
          DosCaseMap       DOSCALLS.391, &
          DosGetCollate    DOSCALLS.392, &
          DosGetCtryInfo   DOSCALLS.393, &
          DosGetDBCSEv     DOSCALLS.394, &
# 32
          DosQueryCtryInfo DOSCALLS.395, &
          DosQueryDBCSEnv DOSCALLS.396,  &
          DosMapCase DOSCALLS.397,       &
          DosQueryCollate DOSCALLS.398

EXPORTS = &
# 16
          DosCaseMap.1, &
          DosGetCollate.2, &
          DosGetCtryInfo.3, &
          DosGetDBCSEv.4, &
# 32
          DosQueryCtryInfo.5, &
          DosQueryDBCSEnv.6,  &
          DosMapCase.7,       &
          DosQueryCollate.8
DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
