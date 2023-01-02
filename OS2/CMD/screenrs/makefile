#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = screenrs
TRGT = $(PROJ).exe
DESC = Utility to set screen resolution in INI file
#defines object file names in format objname.$(O)
srcfiles = $(p)screenrs$(e)
# defines additional options for C compiler
ADD_COPT    = -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
UNI2H = 1

!include $(%ROOT)tools/mk/appsos2_cmd.mk
