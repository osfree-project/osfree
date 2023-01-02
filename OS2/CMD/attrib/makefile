#
# A Makefile for ATTRIB
# (c) osFree project,
#

PROJ = attrib
TRGT = $(PROJ).exe
DESC = Changes file attributes
#defines object file names in format objname.$(O)
srcfiles = $(p)attrib$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
UNI2H = 1

!include $(%ROOT)tools/mk/appsos2_cmd.mk
