#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

!include $(%ROOT)tools/mk/dirs.mk

PROJ     = newmbr
TRGT     = $(PROJ).exe
DESC     = Writes new MBR to the first hard disk
srcfiles = $(p)read$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR) -i=$(PATH) -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
UNI2H = 1


!include $(%ROOT)tools/mk/appsos2_cmd.mk

$(PATH)read.obj: $(PATH)mbr.h

$(PATH)mbr.h: $(%ROOT)build$(SEP)bin$(SEP)boot$(SEP)sectors$(SEP)mbr.bin
 @$(REXX) $(MYDIR)mkstrct.cmd $[@ >$^@
