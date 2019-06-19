#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = help
TRGT = $(PROJ).bat
DESC = Show help info
CLEAN_ADD = *.bat

TARGETS  = $(PATH)$(PROJ).bat
DEST     = os2$(SEP)mdos

!include $(%ROOT)/mk/all.mk

$(PATH)$(PROJ).bat: $(MYDIR)$(PROJ).bat
 @$(CP) $< $^@ $(BLACKHOLE)
