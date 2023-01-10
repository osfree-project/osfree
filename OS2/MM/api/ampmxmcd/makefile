#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = ampmxmcd
TRGT     = $(PROJ).dll
DESC     = MCI DLL
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = mciDriverEntry MMPM.155
EXPORTS = mciDriverEntry

DEST    = mmos2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk


