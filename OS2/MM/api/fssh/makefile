#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = fssh
DESC     = AudioSh
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = _FSSHInitTerm MMPM.196
EXPORTS = _FSSHInitTerm.1

DEST    = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk


