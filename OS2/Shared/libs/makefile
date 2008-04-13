#
# A main Makefile for osFree Command Line tools project
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

# before build command line tools build "shared" libraries
DIRS=zlib lpng JpegLib GDLib GBM libtiff pdcurses

!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
 $(MAKE) TARGET=$^@ -h subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
