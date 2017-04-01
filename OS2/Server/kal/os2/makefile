#
#
#
#


PROJ     = kal
TRGT     = $(PROJ).dll
DESC     = Kernel Abstraction Layer API
srcfiles = $(p)stub$(e)
ADD_COPT = -i=$(MYDIR)..
ADD_LINKOPT  = op nod lib clib3r.lib,$(BLD)lib$(SEP)clibext.lib,$(BLD)lib$(SEP)os2386.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
EXPORTS = KalInit.1, &
          DosWrite.2, &
          DosFSCtl.3, &
          DosExit.4, &
          DosQuerySysInfo.5
UNI2H   = 1

DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
