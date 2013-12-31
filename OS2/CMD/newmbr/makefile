#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

!include $(%ROOT)/mk/dirs.mk

PROJ     = newmbr
DESC     = Writes new MBR to the first hard disk
srcfiles = $(p)read$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR) -i=$(PATH) -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
UNI2H = 1

DEST = os2

!include $(%ROOT)/mk/appsos2_cmd.mk

$(PATH)read.obj: $(PATH)mbr.h

$(PATH)mbr.h: $(ROOT)bin$(SEP)boot$(SEP)sectors$(SEP)mbr.bin
 @$(REXX) $(MYDIR)mkstrct.cmd >$^@
