#
# A Makefile for jpgio.dll
# (c) osFree project,
# author, date
#

PROJ     = mmioMP3
TRGT     = $(PROJ).dll
DESC     = MP3 Format I/O Procedure
srcfiles = $(p)mmioMP3$(e) $(p)xing$(e)
ADD_COPT = -i=$(MYDIR) -i=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)..$(SEP)Shared$(SEP)libs$(SEP)libmad &
           -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until mmos2 .uni modules will be ready
ADD_LINKOPT  = lib libz.lib, mmpm2.lib, libmad.lib &
               segment type DATA nonshared
UNI2H    = 1
DLL      = 1
DLLOPT   = initinstance
OPTIONS  = manyautodata
EXPORTS  = IOProc_Entry.1
STACKSIZE = 65536

DEST     = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
