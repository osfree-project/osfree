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
EXPORTS = KalInit.1, &
          DosWrite.2, &
          DosFSCtl.3, &
          DosExit.4, &
          DosQuerySysInfo.5

DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
