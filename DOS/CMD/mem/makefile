#
# A Makefile for MEM
# (c) osFree project,
# author, date
#

PROJ = mem
TRGT = $(PROJ).exe
DESC = Control file attributes
#defines object file names in format objname.$(O)
srcfiles = $(p)mem$(e) $(p)prf$(e)
# defines additional options for C compiler
ADD_COPT  = -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT = lib $(BLD)lib$(SEP)doscmd_shared.lib

!include $(%ROOT)/mk/appsdos.mk
