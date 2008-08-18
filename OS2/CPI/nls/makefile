#
# A Makefile for nls.dll
# (c) osFree project,
# author, date
#

PROJ     = nls
DESC     = National Language Support API
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = DOSCASEMAP DOSCALLS.391,       &
          DOSGETCOLLATE DOSCALLS.392,    &
          DOSGETCTRYINFO DOSCALLS.393,   &
          DOSGETDBCSEV DOSCALLS.394,     &
          DosQueryCtryInfo DOSCALLS.395, &
          DosQueryDBCSEnv DOSCALLS.396,  &
          DosMapCase DOSCALLS.397,       &
          DosQueryCollate DOSCALLS.398
EXPORTS = DOSCASEMAP.1,       &
          DOSGETCOLLATE.2,    &
          DOSGETCTRYINFO.3,   &
          DOSGETDBCSEV.4,     &
          DosQueryCtryInfo.5, &
          DosQueryDBCSEnv.6,  &
          DosMapCase.7,       &
          DosQueryCollate.8
DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
