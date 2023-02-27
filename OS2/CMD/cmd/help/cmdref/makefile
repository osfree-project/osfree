# $Id: makefile,v 1.2 2004/08/16 04:51:11 prokushev Exp $

PROJ = cmdref
TRGT = $(PROJ).inf
DEST    = os2$(SEP)book

!include $(%ROOT)tools/mk/appsos2_cmd.mk
!include $(MYDIR)..$(SEP)..$(SEP)port$(SEP)4os2.mk

$(PATH)$(PROJ).inf: $(PORT_BASE)4os2.ipf
 @$(SAY) IPFC     $^. $(LOG)
 $(verbose)$(HC) -i $[@ -o $^@ $(LOG2)

#$(PATH)$(PROJ).inf:

TARGETS = $(PATH)$(PROJ).inf
