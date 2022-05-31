#
# A Makefile for XCOPY
# (c) osFree project,
# author, date
#

PROJ = xcopy
TRGT = $(PROJ).exe
DESC = Extended COPY tool
#defines object file names in format objname.$(O)
srcfiles = $(p)xcopy$(e) $(p)prf$(e)
# defines additional options for C compiler
STACKSIZE = 40000
ADD_COPT  = -ml -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT = lib $(BLD)lib$(SEP)doscmd_shared.lib

!include $(%ROOT)/mk/appsdos.mk
