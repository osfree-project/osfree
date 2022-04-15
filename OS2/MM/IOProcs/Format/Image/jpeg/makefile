#
# A Makefile for jpgio.dll
# (c) osFree project,
# author, date
#

PROJ     = mmiojpeg
TRGT     = $(PROJ).dll
DESC     = JPEG Format I/O Procedure
srcfiles = $(p)jpgfunc$(e) $(p)jpgproc$(e) $(p)debug$(e)
ADD_COPT = -i=..$(SEP)include -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until mmos2 .uni modules will be ready
ADD_LINKOPT  = lib gbm.lib, libtiff.lib, &
               libjpeg.lib, libpng.lib, libz.lib, ojpeg.lib &
               segment type DATA shared
UNI2H    = 1
DLL      = 1
DLLOPT   = initglobal
OPTIONS  = manyautodata
EXPORTS  = IOProc_Entry.1
# UNI2H    = 1
DEST     = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
