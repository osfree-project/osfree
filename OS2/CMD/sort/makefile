#
# A Makefile for FIND
# (c) osFree project,
# author, date
#

PROJ = sort
DESC = Sort strings in file
#defines object file names in format objname.$(O)
srcfiles = $(p)sort$(e) $(p)kitten$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR) -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe

!include $(%ROOT)/mk/appsos2.mk
