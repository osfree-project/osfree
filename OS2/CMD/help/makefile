#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = help
DESC = Show help info
CLEAN_ADD = *.cmd

!include $(%ROOT)/mk/all.mk

TARGETS = $(PATH)$(PROJ).cmd
DEST    = $(FILESDIR)$(SEP)os2

$(PATH)$(PROJ).cmd: $(MYDIR)$(PROJ).cmd
 $(CP) $< $^@
