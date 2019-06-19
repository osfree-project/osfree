#
# A Makefile for SUBST
# (c) osFree project,
# author, date
#

PROJ = subst
TRGT = $(PROJ).exe
DESC = Substitute path to drive letter
#defines object file names in format objname.$(O)
srcfiles = $(p)subst$(e)
# defines additional options for C compiler
ADD_COPT=-i=$(MYDIR)..$(SEP)..$(SEP)include
ADD_LINKOPT = lib dos_shared.lib libpath $(ROOT)build$(SEP)lib

!include $(%ROOT)/mk/appsdos.mk
