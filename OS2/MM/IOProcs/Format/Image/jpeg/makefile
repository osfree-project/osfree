#
# A Makefile for jpgio.dll
# (c) osFree project
#

PROJECT        = mmiojpeg
DESCRIPTION    = JPEG Format I/O Procedure
TARGET_CLASS   = LIBRARY
TARGET_PACKAGE = OS2-MM
ADD_COPT = -i=..$(SEP)include -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until mmos2 .uni modules will be ready
ADD_LINKOPT  += lib gbm.lib, libtiff.lib, &
               libjpeg.lib, libpng.lib, libz.lib, ojpeg.lib &
               segment type DATA shared
UNI2H    = 1
DLLOPT   = initglobal
OPTIONS  = manyautodata
EXPORTS  = IOProc_Entry.1
DEST     = mmos2$(SEP)dll

!include $(%ROOT)tools/mk/build.mk
