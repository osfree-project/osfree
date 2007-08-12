#
# A Makefile for FAT filesystem
# (c) osFree project,
# valerius, 2006/10/30
#

!include ../../build.conf
!include ../../mk/site.mk

DIRS = util mini ifs

!include ../../mk/bootseq.mk

all: .SYMBOLIC
 $(MAKE) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) TARGET=$^@ subdirs
