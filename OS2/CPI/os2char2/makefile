#
# A Makefile for os2char2.dll
# (c) osFree project,
# author, date
#

PROJ        = os2char2
TRGT        = $(PROJ).dll
DESC        = 32TEXT API 
srcfiles    = $(p)os2char$(e)
#ADD_COPT    = -i=..$(SEP)include
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
#IMPORTS = 

#EXPORTS = 

DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
