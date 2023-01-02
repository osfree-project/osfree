#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = xcopy
TRGT = $(PROJ).exe
DESC = eXtended copy tool
#defines object file names in format $(p)objname$(e)
srcfiles = $(p)xcopy$(e) $(p)verbose$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
UNI2H = 1

!include $(%ROOT)tools/mk/appsos2_cmd.mk
