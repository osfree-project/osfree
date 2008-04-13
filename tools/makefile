#
# A Makefile for OS/3 build tools
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

DIRS = uniapi shared genext2fs qemu-img ltools awk renmodul &
       rexxwrap mkmsgf # mapsym # libmmap hlldump

!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

install: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 #$(CLEAN_CMD)
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
