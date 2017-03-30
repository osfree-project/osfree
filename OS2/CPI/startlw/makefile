#
# A Makefile for startlw.dll
# (c) osFree project,
# author, date
#

PROJ        = startlw
DESC        = Lazy write API
OBJS        = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT    = -i=..\include
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = STARTLAZYWRITER DOSCALLS.413
EXPORTS = STARTLAZYWRITER.1 RESIDENT
DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
