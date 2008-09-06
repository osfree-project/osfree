#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = PMWindow
DESC = Control utility for ANSI
#defines object file names in format objname.$(O)
srcfiles = $(p)fs_pmdev$(e)
# defines additional options for C compiler
ADD_COPT    = -i=..$(SEP)include -bd
ADD_LIB     = cmd_shared.lib,all_shared.lib
ADD_LINKOPT = lib os2386.lib lib clib3r.lib
DLL = 1
EXPORTS = FPM_DeviceStart=FPM_PMWinStart
DEST = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
