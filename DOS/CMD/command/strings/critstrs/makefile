#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ  = critstrs
DESC  = Control file attributes
#defines object file names in format objname.$(O)
srcfiles = $(p)critstrs$(e) $(p)..$(SEP)porting$(e) # $(p)..$(SEP)tcc2wat$(e)
# defines additional options for C compiler
ADD_COPT = -mc -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)suppl

!include $(%ROOT)/mk/tools.mk

TARGETS = $(PATH)$(PROJ).exe $(PATH)$(PROJ1).exe
