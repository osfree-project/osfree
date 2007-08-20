#
# A main Makefile for osFree Command Line tools project
# (c) osFree project,
# valerius, 2006/10/30
#

!include ../../build.conf
!include ../../mk/site.mk

# before build command line tools build "shared" libraries
DIRS = shared ansi chkdsk eject format label more recover sysinstx &
       cmd ver vol attrib cls comp &
       mkdir tree zip # network bldlevel fdisk syslevel helpmsg

!include ../../mk/bootseq.mk

all: .SYMBOLIC
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
