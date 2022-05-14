#
# A main Makefile for FreeCOM
# (c) osFree project,
# author, date
#


PROJ = command
TRGT = $(PROJ).exe
DESC = Command processor
#defines object file names in format objname.$(O)
srcfiles = $(p)cb_catch$(e) &
           $(p)batch$(e) $(p)cmdtable$(e) $(p)command$(e) $(p)dummies$(e) $(p)expalias$(e) $(p)init$(e) &
           $(p)kswap$(e) $(p)loadhigh$(e) $(p)module$(e) $(p)redir$(e) $(p)ver$(e)  &
           $(p)tcc2wat$(e) # $(p)xms_crit$(e) $(p)xms_brk$(e) $(p)xms_2e$(e) $(p)cswap$(e) 
# defines additional options for C compiler

ADD_COPT    = -ml -dMODEL=l -d2 -db -i=$(PATH)..$(SEP)criter$(SEP)criter1 -i=$(MYDIR)..$(SEP)strings -i=$(MYDIR)..$(SEP)suppl -i=$(MYDIR)..$(SEP)include -i=$(MYDIR) -i=$(MYDIR)..
ADD_ASMOPT  = -ml -dMODEL=l -d2 -db -i=$(PATH)..$(SEP)criter$(SEP)criter1 -i=$(MYDIR)..$(SEP)strings -i=$(MYDIR)..$(SEP)suppl  -i=$(MYDIR)..$(SEP)include -i=$(MYDIR) -i=$(MYDIR).. &
              -i=$(PATH)..$(SEP)strings$(SEP)critstrs
ADD_LINKOPT = debug all libpath $(%ROOT)build$(SEP)lib lib cmd.lib, lib.lib, suppl.lib, strings.lib

!include $(%ROOT)/mk/appsdos.mk

