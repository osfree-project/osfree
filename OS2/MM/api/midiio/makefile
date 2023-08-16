#
# A Makefile for midiio.dll
# (c) osFree project
#

PROJ     = midiio
TRGT     = $(PROJ).dll
DESC     = Indeo IOPROC Intel Indeo(TM) V3.12.702 Copyright(c) Intel Corporation 1993
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
srcfiles = $(p)..$(SEP)shared$(SEP)stub$(e)
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = MIDIIOProc MMPM.160
EXPORTS = MIDIIOProc.1

DEST    = mmos2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk


