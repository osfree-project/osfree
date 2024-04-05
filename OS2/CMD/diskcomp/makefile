#
# A Makefile for DISKCOMP
# (c) osFree project,
#

PROJ = diskcomp
TRGT = $(PROJ).exe
DESC = Disk compare tool
#defines object file names in format $(p)objname$(e)
srcfiles = $(p)diskcomp$(e) $(p)misc$(e) $(p)md5$(e) $(p)drive$(e)
# defines additional options for C compiler
ADD_COPT = -i=. -i=..$(SEP)include
STACK    = 65536
STUB     = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
UNI2H = 1

!include $(%ROOT)tools/mk/appsos2_cmd.mk
!include $(MYDIR)diskcomp.mk

.c: $(PORT_BASE)

.h: $(PORT_BASE)

