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
IMPORTS = DosQueryCtryInfo DOSCALLS.395, &
	  DosQueryDBCSEnv DOSCALLS.396,  &
	  DosMapCase DOSCALLS.397,       &
	  DosQueryCollate DOSCALLS.398
EXPORTS = DosQueryCtryInfo.5, &
	  DosQueryDBCSEnv.6,  &
	  DosMapCase.7,       &
	  DosQueryCollate.8
DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
