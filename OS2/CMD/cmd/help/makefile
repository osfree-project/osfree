# $Id: makefile,v 1.2 2004/08/16 04:51:11 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

ROOT = $(%ROOT)

DIR = $(FILESDIR)$(SEP)os2$(SEP)book
PROJ = cmdref
PROJ2 = external

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ).inf $(DIR)$(SEP)$(PROJ2).inf

# Clean target
.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
