#
# A Makefile for jpgio.dll
# (c) osFree project,
# author, date
#

PROJ     = mmioMP3
DESC     = JPEG Format I/O Procedure
srcfiles = $(p)mmioMP3$(e) $(p)xing$(e)
ADD_COPT = -i=$(MYDIR) -i=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)..$(SEP)Shared$(SEP)libs$(SEP)libmad -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until mmos2 .uni modules will be ready
ADD_LINKOPT  = lib os2386.lib, clib3r.lib, libz.lib, mmpm2.lib, libmad.lib &
               segment type DATA nonshared
DLL      = 1
DLLOPT   = initinstance
OPTIONS  = manyautodata
EXPORTS  = IOProc_Entry.1
STACKSIZE = 65536

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
