#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = audiomcd
DESC     = MCI DLL
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = mciDriverEntry MMPM.153
EXPORTS = mciDriverEntry

DEST    = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk


