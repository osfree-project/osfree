#
# A Makefile for COMP
# (c) osFree project,
# author, date
#

PROJ = comp
TRGT = $(PROJ).exe
DESC = Compare tool
#defines object file names in format objname.$(O)
srcfiles = $(p)comp$(e)
# defines additional options for C compiler
# comp.exe doesn't exist for DOS. For now it is comp.com...
#STUB     = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)$(PROJ).exe
ADD_COPT = -i=$(MYDIR)..$(SEP)include
UNI2H = 0


!include $(%ROOT)tools/mk/appsos2v1.mk
