# $Id: makefile,v 1.4 2004/08/16 04:51:09 prokushev Exp $

PROJ = bldlevel
#TRGT = $(PROJ).cmd
DESC = Build level display facility
CLEAN_ADD = *.cmd

!include $(%ROOT)/mk/all.mk

TARGETS = $(PATH)$(PROJ).cmd

$(PATH)$(PROJ).cmd: $(MYDIR)$(PROJ).rexx
 @$(CP) $< $^@ $(BLACKHOLE)
