#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = dskxtrct
TRGT = $(PROJ).exe
DESC = Extract diskette images
#defines object file names in format objname.$(O)
srcfiles = $(p)stringe$(e) $(p)dskxtrct$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)stringe
ADD_LINKOPT = lib plib3r.lib
# STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
UNI2H = 1

!include $(%ROOT)/mk/appsos2_cmd.mk
