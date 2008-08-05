#
# A main Makefile for osFree Command Line tools project
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

# before build command line tools build "shared" libraries
DIRS=zlib lpng JpegLib GDLib GBM libtiff pdcurses glib libIDL

!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
