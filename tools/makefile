#
# A Makefile for OS/3 boot sequence project
# micro IIF drivers
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

DIRS = shared hlldump genext2fs qemu-img ltools awk renmodul rexxwrap mkmsgf # mapsym libmmap

!include $(%ROOT)/mk/bootseq.mk

all: .SYMBOLIC
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

install: .SYMBOLIC
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 #$(CLEAN_CMD)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
