#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = ansi
TRGT = $(PROJ).exe
DESC = Control utility for ANSI
#defines object file names in format objname.$(O)
srcfiles = $(p)ansi$(e)
# defines additional options for C compiler
ADD_COPT    = -i=$(MYDIR)..$(SEP)include
NODEFAULTLIBS = 1
UNI2H = 1
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

!include $(%ROOT)/mk/appsos2_cmd.mk
