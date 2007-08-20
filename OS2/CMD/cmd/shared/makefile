# $Id: makefile,v 1.3 2004/08/16 04:51:12 prokushev Exp $
#

!include ../../../../build.conf
!include ../../../../mk/site.mk

ADD_COPT = -i=..$(SEP)include
32_BITS  = 1

!include ../../../../mk/cmd.mk

all: istring.obj

# Clean all
.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
