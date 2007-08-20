# $Id: makefile,v 1.2 2004/08/16 04:51:11 prokushev Exp $

!include ../../../../build.conf
!include ../../../../mk/site.mk

ROOT = ..$(SEP)..$(SEP)..$(SEP)..

DIR = $(ROOT)$(SEP)bin$(SEP)os2$(SEP)nls$(SEP)book
PROJ = cmdref
PROJ2 = external

!include ../../../../mk/cmd.mk

all: $(DIR)$(SEP)$(PROJ).inf $(DIR)$(SEP)$(PROJ2).inf

# Clean target
.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
