#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ  = fixstrs
TRGT  = $(PROJ).exe
DESC  = Control file attributes
DIRS  = critstrs
#defines object file names in format objname.$(O)
srcfiles = $(p)fixstrs$(e) $(p)porting$(e) # $(p)..$(SEP)tcc2wat$(e)
# defines additional options for C compiler
ADD_COPT =  -d2 -db -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)suppl
ADD_LINKOPT = debug all lib suppl.lib
LANG = russian
LNG = $(MYDIR)$(LANG).lng
DEST = none

!include $(%ROOT)/mk/tools.mk

TARGETS = $(PATH)$(PROJ).exe $(PATH)strings.h subdirs lib
TARGET = all

$(PATH)strings.h : $(MYDIR)default.lng $(PATH)$(PROJ).exe
 $(PATH)fixstrs.exe /lib /dir $(PATH) $(LNG)

lib: $(PATH)makefile.mk
 $(MAKE) $(MAKEOPT) -f $< lib
