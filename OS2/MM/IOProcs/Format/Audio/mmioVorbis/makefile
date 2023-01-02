#
# A Makefile for jpgio.dll
# (c) osFree project,
# author, date
#

PROJ     = mmiovorb
TRGT     = $(PROJ).dll
DESC     = MP3 Format I/O Procedure
srcfiles = $(p)mmioVorbis$(e)
ADD_COPT = -i=$(MYDIR) -i=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)..$(SEP)Shared$(SEP)libs$(SEP)libogg$(SEP)include &
           -i=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)..$(SEP)Shared$(SEP)libs$(SEP)libvorbis$(SEP)include &
           -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until mmos2 .uni modules will be ready
ADD_LINKOPT  = lib libz.lib, mmpm2.lib, libogg.lib, libvorbis.lib &
               segment type DATA nonshared
UNI2H    = 1
DLL      = 1
DLLOPT   = initinstance
OPTIONS  = manyautodata
EXPORTS  = IOProc_Entry.1
STACKSIZE = 1048576

DEST     = mmos2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
