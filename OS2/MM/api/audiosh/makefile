#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = audiosh
TRGT     = $(PROJ).dll
DESC     = AudioSh
srcfiles = $(p)..$(SEP)shared$(SEP)stub$(e)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = _ADSHInitTerm MMPM.207
EXPORTS = _ADSHInitTerm.1

DEST    = mmos2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk


