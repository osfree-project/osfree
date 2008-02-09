#
# A main Makefile for osFree Command Line tools project
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

# before build command line tools build "shared" libraries
DIRS = shared minicmd ansi chkdsk eject format label more recover sysinstx &
       cmd ver vol attrib cls comp bldlevel &
       mkdir tree zip # network fdisk syslevel helpmsg

!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
