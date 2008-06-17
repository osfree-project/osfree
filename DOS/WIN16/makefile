#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

# First directory must be UNI which produces C/C++ header files

DIRS = samples lzexpand


!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(MKDIR) $(FILESDIR)$(SEP)os2
 $(MKDIR) $(FILESDIR)$(SEP)os2$(SEP)dll
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
