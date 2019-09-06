#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ  = fixstrs
DESC  = Control file attributes
DIRS  = critstrs
#defines object file names in format objname.$(O)
srcfiles = $(p)fixstrs$(e) $(p)porting$(e) # $(p)..$(SEP)tcc2wat$(e)
# defines additional options for C compiler
!ifeq %OS LINUX
ADD_COPT = -dLINUX
!endif
ADD_COPT += -dMODEL=l -d2 -db -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)suppl
ADD_LINKOPT = debug all 
LANG = russian
LNG = $(MYDIR)$(LANG).lng

DEST    = ..$(SEP)build$(SEP)lib
INSTALL = strings.lib

!include $(%ROOT)/mk/tools.mk

TARGETS = $(PATH)$(PROJ).exe $(PATH)strings.h subdirs $(PATH)strings.lib

$(PATH)$(PROJ).exe:

$(PATH)strings.h : $(MYDIR)default.lng $(PATH)$(PROJ).exe
 @$(PATH)$(PROJ).exe /lib /dir $(PATH) $(LNG)

$(PATH)strings.lib: $(PATH)makefile.mk
 @$(MAKE) $(MAKEOPT) -f $< lib
