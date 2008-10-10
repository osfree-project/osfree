#
# A Makefile for lvm.lib
# (c) osFree project,
# author, date
#

PROJ     = format_text
srcfiles = $(p)archive$(e) $(p)archiver$(e) $(p)export$(e) $(p)flags$(e) $(p)format-text$(e) $(p)import$(e) &
           $(p)import_vsn1$(e) $(p)tags$(e) $(p)text_label$(e)
ADD_COPT = -i=$(MYDIR)..$(SEP)datastruct -i=$(MYDIR)..$(SEP)misc -i=$(MYDIR)..$(SEP)config -i=$(MYDIR)..$(SEP)commands &
           -i=$(MYDIR)..$(SEP)log -i=$(MYDIR)..$(SEP)metadata -i=$(MYDIR)..$(SEP)device -i=$(MYDIR)..$(SEP)uuid &
           -i=$(MYDIR)..$(SEP)display -i=$(MYDIR)..$(SEP)format_text -i=$(MYDIR)..$(SEP)activate -i=$(MYDIR)..$(SEP)mm &
           -i=$(MYDIR)..$(SEP)locking -i=$(MYDIR)..$(SEP)cache -i=$(MYDIR)..$(SEP)label -i=$(MYDIR)..$(SEP)filters &
           -i=$(MYDIR)..$(SEP)activate -bd
TARGETS  = $(PATH)$(PROJ).lib
DEST =

!include $(%ROOT)/mk/libsos2.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
