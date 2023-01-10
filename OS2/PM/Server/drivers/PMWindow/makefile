#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = pmwindow
TRGT = $(PROJ).dll
DESC = PM Window user interface backend
#defines object file names in format objname.$(O)
srcfiles = $(p)Fs_PMdev$(e)
# defines additional options for C compiler
ADD_COPT    = -i=..$(SEP)include -i=..$(SEP).. -bm -od -mf -sg -s
ADD_LIB     = cmd_shared.lib,all_shared.lib
# ADD_LINKOPT = lib os2386.lib lib clib3r.lib
UNI2H   = 1
DLL     = 1
EXPORTS = FPM_DeviceStart

DEST = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_fpm.mk
