#
# A Makefile for os2char2.dll
# (c) osFree project,
# author, date
#

PROJ        = os2char2
TRGT        = $(PROJ).dll
DESC        = os2char2 (IBM 32-bit vio/kbd/mou) API
OBJS        = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT    = -i=..\include
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = 
EXPORTS = 
DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
