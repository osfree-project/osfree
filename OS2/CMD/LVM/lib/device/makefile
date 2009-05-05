#
# A Makefile for lvm.lib
# (c) osFree project,
# author, date
#

PROJ     = device
srcfiles = $(p)dev-cache$(e) $(p)dev-io$(e) $(p)dev-md$(e) $(p)device$(e)
ADD_COPT = -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)os2 &
           -i=$(MYDIR)..$(SEP)datastruct -i=$(MYDIR)..$(SEP)misc -i=$(MYDIR)..$(SEP)config -i=$(MYDIR)..$(SEP)commands &
           -i=$(MYDIR)..$(SEP)log -i=$(MYDIR)..$(SEP)metadata -i=$(MYDIR)..$(SEP)device -i=$(MYDIR)..$(SEP)uuid &
           -i=$(MYDIR)..$(SEP)display -i=$(MYDIR)..$(SEP)format_text -i=$(MYDIR)..$(SEP)activate -i=$(MYDIR)..$(SEP)mm &
           -i=$(MYDIR)..$(SEP)locking -i=$(MYDIR)..$(SEP)cache -i=$(MYDIR)..$(SEP)label -i=$(MYDIR)..$(SEP)filters &
           -i=$(MYDIR)..$(SEP)activate -bd
TARGETS  = $(PATH)$(PROJ).lib
DEST =

!include $(%ROOT)/mk/libsos2.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
