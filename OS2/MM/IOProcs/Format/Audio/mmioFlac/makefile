#
# A Makefile for jpgio.dll
# (c) osFree project,
# author, date
#

PROJ     = mmioflac
TRGT     = $(PROJ).dll
DESC     = FLAC Format I/O Procedure
srcfiles = $(p)main$(e) $(p)mmioFLAC$(e)
ADD_COPT = -i=$(MYDIR) -i=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)..$(SEP)Shared$(SEP)libs$(SEP)libflac$(SEP)include &
           -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until mmos2 .uni modules will be ready
ADD_LINKOPT  = lib libz.lib, mmpm2.lib, libflac.lib &
               segment type DATA nonshared
UNI2H    = 1
DLL      = 1
DLLOPT   = initinstance
OPTIONS  = manyautodata
EXPORTS  = IOProc_Entry.1
STACKSIZE = 65536

DEST     = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
