#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

!include ../build.conf
!include ../mk/site.mk

DIRS = fat ext2fs hpfs jfs util

!include ../mk/bootseq.mk

all: .SYMBOLIC
 $(MAKE) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) TARGET=$^@ subdirs
