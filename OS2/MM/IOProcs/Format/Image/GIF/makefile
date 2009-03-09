#
# A Makefile for gifio.dll
# (c) osFree project,
# author, date
#

PROJ     = mmioGIF
DESC     = GIF Format I/O Procedure
srcfiles = $(p)gifproc$(e)
ADD_COPT = -i=..$(SEP)include -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until mmos2 .uni modules will be ready
ADD_LINKOPT  = lib os2386.lib, clib3r.lib, gbm.lib, libtiff.lib, &
               libjpeg.lib, png.lib, libz.lib, ojpeg.lib &
               segment type DATA shared
DLL      = 1
DLLOPT   = initglobal
OPTIONS  = manyautodata
EXPORTS  = IOProc_Entry.1
DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
