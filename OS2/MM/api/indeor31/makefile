#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = indeor31
TRGT     = $(PROJ).dll
DESC     = Indeo IOPROC Intel Indeo(TM) V3.12.702 Copyright(c) Intel Corporation 1993
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = Indeo31Proc INDDEC32.1
EXPORTS = Indeo31Proc.1

DEST    = mmos2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk


