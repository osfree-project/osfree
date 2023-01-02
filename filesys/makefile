#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)tools/mk/site.mk

DIRS = fat ext2fs hpfs jfs

!include $(%ROOT)tools/mk/bootseq.mk

all: .SYMBOLIC
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
