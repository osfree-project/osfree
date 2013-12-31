#
# A Makefile for emitlnk.dll
# (c) osFree project,
# author, date
#

PROJ     = emitdef
DESC     = Definition File Emittter for SOM Compiler
srcfiles = $(p)emitpas$(e) $(p)pas$(e)
ADD_COPT = -i=$(%ROOT)$(SEP)include$(SEP)os3
ADD_LINKOPT  = lib os2386.lib, clib3r.lib lib somtk.lib &
               segment type DATA shared
DLL      = 1
DLLOPT   = INITINSTANCE TERMINSTANCE
OPTIONS  = manyautodata
EXPORTS  = emit

!include $(%ROOT)/mk/appsos2_cmd.mk
