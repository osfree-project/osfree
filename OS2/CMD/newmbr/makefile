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

!include $(%ROOT)/mk/appsos2.mk

$(PATH)read.obj: $(PATH)mbr.h

$(PATH)mbr.h: $(ROOT)bin$(SEP)boot$(SEP)sectors$(SEP)mbr.bin
 @$(MYDIR)mkstrct.cmd >$^@
