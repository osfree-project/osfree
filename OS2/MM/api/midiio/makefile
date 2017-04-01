#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = midiio
TRGT     = $(PROJ).dll
DESC     = Indeo IOPROC Intel Indeo(TM) V3.12.702 Copyright(c) Intel Corporation 1993
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = MIDIIOProc MMPM.160
EXPORTS = MIDIIOProc.1

DEST    = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk


