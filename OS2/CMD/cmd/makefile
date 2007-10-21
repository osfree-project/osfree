# $Id: makefile,v 1.3 2004/08/16 04:51:10 prokushev Exp $
#
# A makefile for osFree command interpreter
# (c) osFree project
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

# Add other targets here upon their maturity
#   Moved the help dir to last, in that case the tools for inf files
#   is not available.
DIRS = shared cmd keystack shralias option2 jpos2dll help vdm2os2

!include $(%ROOT)/mk/all.mk


all: .SYMBOLIC
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
