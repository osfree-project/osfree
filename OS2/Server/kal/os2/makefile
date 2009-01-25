#
#
#
#


PROJ     = kal
DESC     = Kernel Abstraction Layer API
srcfiles = $(p)stub$(e)
ADD_COPT = -i=$(MYDIR)..
ADD_LINKOPT  = op nod lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
EXPORTS = DosFSCtl.1, &
          DosWrite.2, &
          DosExit.3, &
          DosQuerySysInfo.4

DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
