#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = label
TRGT = $(PROJ).exe
DESC = Volume label control utility
#defines object file names in format objname.$(O)
srcfiles = $(p)label$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
UNI2H = 1

!include $(%ROOT)/mk/appsos2_cmd.mk
