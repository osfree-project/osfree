#
# A Makefile for jpgio.dll
# (c) osFree project,
# author, date
#

PROJ     = mmioGIF
DESC     = GIF Format I/O Procedure
srcfiles = $(p)gifproc$(e)
ADD_COPT = -i=..$(SEP)include
ADD_LINKOPT  = lib os2386.lib, clib3r.lib, gbm.lib, libtiff.lib, jpeglib.lib, lpng.lib, libz.lib
ADD_LINKOPT  = lib os2386.lib, clib3r.lib, gbm.lib, libtiff.lib, &
               libjpeg.lib, libpng.lib, libz.lib &
               segment type DATA shared
DLL      = 1
DLLOPT   = initglobal
OPTIONS  = manyautodata
EXPORTS  = IOProc_Entry.1

!include $(%ROOT)/mk/appsos2.mk
