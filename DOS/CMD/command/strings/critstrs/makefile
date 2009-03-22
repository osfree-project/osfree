#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ  = critstrs
DESC  = Control file attributes
#defines object file names in format objname.$(O)
srcfiles = $(p)critstrs$(e) $(p)..$(SEP)porting$(e) # $(p)..$(SEP)tcc2wat$(e)
# defines additional options for C compiler
ADD_COPT =  -d2 -db -i=$(MYDIR)..$(SEP)..$(SEP)include -i=$(MYDIR)..$(SEP)..$(SEP)suppl
ADD_LINKOPT = debug all lib suppl.lib
LANG = russian
LNG = $(MYDIR)..$(SEP)$(LANG).err
DEST = none

!include $(%ROOT)/mk/tools.mk

TARGETS = $(PATH)$(PROJ).exe $(PATH)strings.err

.SUFFIXES: .err

.err: $(PATH)

$(PATH)strings.err: $(MYDIR)..$(SEP)default.err $(PATH)critstrs.exe
 $(CP) $[@ $(PATH)
 $(PATH)critstrs.exe /dir $(PATH) $(LNG)
