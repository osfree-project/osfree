#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

DIRS = bootsec loader

!include $(%ROOT)/mk/bootseq.mk

all: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
