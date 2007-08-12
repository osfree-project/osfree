#
# A main Makefile for OS/3 boot sequence project.
# Utilities.
# (c) osFree project,
# valerius, 2006/10/30
#

!include ../../build.conf
!include ../../mk/site.mk

DIRS = # sysinstx recover format chkdsk

!include ../../mk/bootseq.mk

all: .SYMBOLIC
 $(MAKE) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) TARGET=$^@ subdirs
