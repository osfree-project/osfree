#
# A Makefile for kal.dll
# (c) osFree project,
# author, date
#

PROJ     = kal
DESC     = Kernel Abstracion Layer API
srcfiles = $(p)file$(e) $(p)..$(SEP)shared$(SEP)stub$(e)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
# all FS related API works via this API
EXPORTS = DosFSCtl.1,        &
          DosPutMessage2.8,  &
          DosQuerySysInfo.9, &
          DosExit.10
DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
