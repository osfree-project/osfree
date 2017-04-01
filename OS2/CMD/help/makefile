#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = help
TRGT = $(PROJ).cmd
DESC = Show help info
CLEAN_ADD = *.cmd

!include $(%ROOT)/mk/all.mk

$(PATH)$(PROJ).cmd: $(MYDIR)$(PROJ).cmd
 @$(CP) $< $^@ $(BLACKHOLE)
