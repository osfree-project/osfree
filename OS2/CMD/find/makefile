#
# A Makefile for FIND
# (c) osFree project,
# author, date
#

PROJ = find
TRGT = $(PROJ).exe
DESC = Find string in file
#defines object file names in format objname.$(O)
srcfiles = $(p)find$(e) $(p)find_str$(e) $(p)kitten$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR) -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
UNI2H = 1

!include $(%ROOT)tools/mk/appsos2_cmd.mk
!include $(MYDIR)find.mk

.c: $(PORT_BASE)src;$(PORT_BASE)kitten;

.h: $(PORT_BASE)src;$(PORT_BASE)kitten;
