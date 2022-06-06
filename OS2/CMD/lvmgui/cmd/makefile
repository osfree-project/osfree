#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = lvmgui
TRGT = $(PROJ).cmd
DESC = GUI frontent for LVM
CLEAN_ADD = *.cmd

!include $(%ROOT)/mk/all.mk

$(PATH)$(PROJ).cmd: $(MYDIR)$(PROJ).cmd
 @$(CP) $< $^@ $(BLACKHOLE)
