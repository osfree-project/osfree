#
# A Makefile for lvm.lib
# (c) osFree project,
# author, date
#

PROJ     = datastruct
srcfiles = $(p)bitset$(e) $(p)hash$(e)
ADD_COPT = -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)os2 &
           -i=$(MYDIR) -i=$(MYDIR)..$(SEP)..$(SEP)include -bd
TARGETS  = $(PATH)$(PROJ).lib
DEST     = none

!include $(%ROOT)/mk/libsos2.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library

.c: $(MYDIR)..

.c: $(MYDIR)
