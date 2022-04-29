#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ  = mmiogbm
TRGT  = $(PROJ).dll
PROJ0 = mmiogbm
DESC  = GBM IOProc generator
#defines object file names in format objname.$(O)
srcfiles = $(p)mmioproc$(e)
# defines additional options for C compiler
ADD_COPT    = -i=$(%ROOT)include$(SEP)os3$(SEP)gbm -i=$(%WATCOM)$(SEP)h$(SEP)os2
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
ADD_LINKOPT  = lib mmpm2.lib, gbm.lib, libtiff.lib, libjpeg.lib, libpng.lib, libz.lib, ojpeg.lib &
               segment type DATA shared
UNI2H    = 1
DLL = 1

!include $(%ROOT)/mk/appsos2_cmd.mk
