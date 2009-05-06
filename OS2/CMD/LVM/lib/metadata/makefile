#
# A Makefile for lvm.lib
# (c) osFree project,
# author, date
#

PROJ     = metadata
srcfiles = $(p)metadata$(e) $(p)lv_manip$(e) $(p)merge$(e) $(p)mirror$(e) $(p)pv_manip$(e) $(p)pv_map$(e) &
           $(p)segtype$(e) $(p)snapshot_manip$(e)
ADD_COPT = -i=$(MYDIR)..$(SEP)datastruct -i=$(MYDIR)..$(SEP)misc -i=$(MYDIR)..$(SEP)config -i=$(MYDIR)..$(SEP)commands &
           -i=$(MYDIR)..$(SEP)log -i=$(MYDIR)..$(SEP)metadata -i=$(MYDIR)..$(SEP)device -i=$(MYDIR)..$(SEP)uuid &
           -i=$(MYDIR)..$(SEP)display -i=$(MYDIR)..$(SEP)format_text -i=$(MYDIR)..$(SEP)activate -i=$(MYDIR)..$(SEP)mm &
           -i=$(MYDIR)..$(SEP)locking -i=$(MYDIR)..$(SEP)cache -i=$(MYDIR)..$(SEP)label -i=$(MYDIR)..$(SEP)filters &
           -i=$(MYDIR)..$(SEP)activate -bd
TARGETS  = $(PATH)$(PROJ).lib
DEST = none

# don't delete lib if exists
!define NODELETE
!include $(%ROOT)/mk/libsos2.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
