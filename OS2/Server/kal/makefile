#
# A Makefile for kal.dll
# (c) osFree project,
# author, date
#

PROJ     = kal
DESC     = Kernel Abstracion Layer API
DIRS     = os2 # linux win32 l4
srcfiles = $(p)file$(e)
ADD_COPT = -i=..\include
ADD_LINKOPT  = op nod lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
# all FS related API works via this API
EXPORTS = DosFSCtl.1,        &
          DosWrite.2,        &
          DosExit.3,         &
          DosQuerySysInfo.4

!include $(%ROOT)/mk/appsos2_cmd.mk

DEST     = os2$(SEP)dll
TARGETS  = subdirs # $(PATH)$(PROJ).exe
