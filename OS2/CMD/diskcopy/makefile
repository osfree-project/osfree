#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

32_BITS = 1
PROJ = diskcopy
TRGT = $(PROJ).exe
DESC = Copy diskette
#defines object file names in format objname.$(O)
srcfiles = $(p)diskcopy$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
UNI2H = 1

!include $(%ROOT)/mk/appsos2.mk
