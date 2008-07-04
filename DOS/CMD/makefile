#
# A main Makefile for osFree Command Line tools project
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk


DIRS =   attrib   &
         help     &
         find     &
         diskcomp &
         label    &
         stub

!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
