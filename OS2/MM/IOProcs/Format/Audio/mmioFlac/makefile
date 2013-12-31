#
# A Makefile for jpgio.dll
# (c) osFree project,
# author, date
#

PROJ     = mmioFLAC
DESC     = JPEG Format I/O Procedure
srcfiles = $(p)main$(e) $(p)mmioFLAC$(e)
ADD_COPT = -i=$(MYDIR) -i=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)..$(SEP)Shared$(SEP)libs$(SEP)libflac$(SEP)include -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until mmos2 .uni modules will be ready
ADD_LINKOPT  = lib os2386.lib, clib3r.lib, libz.lib, mmpm2.lib, libflac.lib &
               segment type DATA nonshared
DLL      = 1
DLLOPT   = initinstance
OPTIONS  = manyautodata
EXPORTS  = IOProc_Entry.1
STACKSIZE = 1048576

DEST     = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
