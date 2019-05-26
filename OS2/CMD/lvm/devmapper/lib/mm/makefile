#
# A Makefile for lvm.lib
# (c) osFree project,
# author, date
#

PROJ     = mm
srcfiles = $(p)dbg_malloc$(e) $(p)pool-debug$(e) $(p)pool-fast$(e) $(p)pool$(e)
ADD_COPT = -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)os2 &
           -i=$(MYDIR) -i=$(MYDIR)..$(SEP)..$(SEP)include -bd
TARGETS  = $(PATH)$(PROJ).lib
DEST     = none

!include $(%ROOT)/mk/libsos2.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library

.c: $(MYDIR)..

.c: $(MYDIR)
