#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ  = fixstrs
DIRS  = critstrs
DESC  = Control file attributes
#defines object file names in format objname.$(O)
srcfiles = $(p)fixstrs$(e) $(p)porting$(e) # $(p)tcc2wat$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)suppl
LNG = $(MYDIR)russian.lng
CFG = $(MYDIR)..$(SEP)config.h

!include $(%ROOT)/mk/tools.mk

TARGETS = $(PATH)$(PROJ).exe subdirs $(PATH)strings.h
TARGET = all

.h: $(PATH)

$(PATH)strings.h: $(MYDIR)default.lng $(PATH)fixstrs.exe
 $(PATH)fixstrs.exe /lib $(LNG)

