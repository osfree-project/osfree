#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ  = fixstrs
DESC  = Control file attributes
#defines object file names in format objname.$(O)
srcfiles = $(p)fixstrs$(e) $(p)porting$(e) # $(p)..$(SEP)tcc2wat$(e)
# defines additional options for C compiler
ADD_COPT =  -d2 -db -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)suppl
ADD_LINKOPT = debug all lib suppl.lib
LNG = $(MYDIR)russian.lng

!include $(%ROOT)/mk/tools.mk

TARGETS = $(PATH)$(PROJ).exe $(PATH)strings.h subdirs
TARGET = all

$(PATH)strings.h : $(MYDIR)default.lng $(PATH)$(PROJ).exe
 $(PATH)fixstrs.exe /lib /dir $(PATH) $(LNG)

