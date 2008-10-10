#
# A Makefile for lvm.lib
# (c) osFree project,
# author, date
#

PROJ     = locking
srcfiles = $(p)cluster_locking$(e) $(p)external_locking$(e) $(p)file_locking$(e) &
           $(p)locking$(e) $(p)no_locking$(e)
ADD_COPT = -i=$(MYDIR)..$(SEP)datastruct -i=$(MYDIR)..$(SEP)misc -i=$(MYDIR)..$(SEP)config -i=$(MYDIR)..$(SEP)commands &
           -i=$(MYDIR)..$(SEP)log -i=$(MYDIR)..$(SEP)metadata -i=$(MYDIR)..$(SEP)device -i=$(MYDIR)..$(SEP)uuid &
           -i=$(MYDIR)..$(SEP)display -i=$(MYDIR)..$(SEP)format_text -i=$(MYDIR)..$(SEP)activate -i=$(MYDIR)..$(SEP)mm &
           -i=$(MYDIR)..$(SEP)locking -i=$(MYDIR)..$(SEP)cache -i=$(MYDIR)..$(SEP)label -i=$(MYDIR)..$(SEP)filters &
           -i=$(MYDIR)..$(SEP)activate -i=$(MYDIR)..$(SEP)..$(SEP)daemons$(SEP)clvmd -bd
TARGETS  = $(PATH)$(PROJ).lib
DEST =

!include $(%ROOT)/mk/libsos2.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
