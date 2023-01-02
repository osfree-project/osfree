#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = undelete
TRGT = $(PROJ).exe
DESC = Restore deleted file
#defines object file names in format objname.$(O)
srcfiles = $(p)undelete$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
UNI2H = 1

!include $(%ROOT)tools/mk/appsos2_cmd.mk
