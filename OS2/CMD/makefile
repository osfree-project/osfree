#
# A main Makefile for osFree Command Line tools project
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

# before build command line tools build "shared" libraries
DIRS = shared ansi &
              attrib &
              bldlevel &
              chkdsk &
              cls &
              cmd &
              comp &
              diskcomp &
              diskcopy &
              eject &
              fdisk &
              fdiskpm &
              find &
              format &
              help &
              helpmsg &
              keyb &
              label &
              lvm &
              lvmgui &
              makeini &
              minicmd &
              mkdir &
              mode &
              more &
              print &
              recover &
              sort &
              sysinstx &
              syslevel &
              tedit &
              tree &
              undelete &
              ver &
              vol &
              xcopy &
              zip # network

!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
