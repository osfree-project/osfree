#
# A Makefile for MORE
# (c) osFree project,
# author, date
#

PROJ = more
TRGT = $(PROJ).exe
DESC = Show text file page by page
#defines object file names in format objname.$(O)
srcfiles = $(p)find95$(e) $(p)l2s95$(e) $(p)lfnapi$(e) $(p)more$(e) &
           $(p)prf$(e) $(p)talloc$(e) $(p)tcdummy$(e)

# defines additional options for linker
STACKSIZE = 40000
ADD_COPT  = -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT = lib $(BLD)lib$(SEP)doscmd_shared.lib

!include $(%ROOT)/mk/appsdos.mk
