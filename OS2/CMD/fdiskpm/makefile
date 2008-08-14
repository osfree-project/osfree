# $Id: makefile,v 1.4 2004/08/16 04:51:09 prokushev Exp $

PROJ = fdiskpm
DESC = Disk volumes manager for PM

!include $(%ROOT)/mk/all.mk

TARGETS = $(PATH)$(PROJ).cmd
DEST    = $(FILESDIR)$(SEP)os2

$(PATH)$(PROJ).cmd: $(MYDIR)$(PROJ).rexx
 $(CP) $< $^@
