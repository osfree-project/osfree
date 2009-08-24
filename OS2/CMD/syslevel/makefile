# $Id: makefile,v 1.3 2003/10/29 11:04:06 prokushev Exp $

PROJ = syslevel
DESC = Display syslevel info
CLEAN_ADD = *.cmd

!include $(%ROOT)/mk/all.mk

TARGETS = $(PATH)$(PROJ).cmd
DEST    = $(FILESDIR)$(SEP)os2

$(PATH)$(PROJ).cmd: $(MYDIR)$(PROJ).cmd
 $(CP) $< $^@
