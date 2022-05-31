#
# A Makefile for FIND
# (c) osFree project,
# author, date
#

PROJ = find
TRGT = $(PROJ).exe
DESC = Find string in file
#defines object file names in format objname.$(O)
srcfiles = $(p)lfnchk$(e) $(p)l2s95$(e) $(p)find95$(e) $(p)find$(e) &
           $(p)find_str$(e) $(p)lfnapi$(e)
# defines additional options for C compiler
ADD_COPT  = -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT = lib $(BLD)lib$(SEP)doscmd_shared.lib

!include $(%ROOT)/mk/appsdos.mk
