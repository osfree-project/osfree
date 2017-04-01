#
# A Makefile for FIND
# (c) osFree project,
# author, date
#

PROJ = sort
TRGT = $(PROJ).exe
DESC = Sort strings in file
#defines object file names in format objname.$(O)
srcfiles = $(p)sort$(e) $(p)kitten$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR) -i=$(MYDIR)..$(SEP)include
#STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
UNI2H = 1

!include $(%ROOT)/mk/appsos2_cmd.mk
