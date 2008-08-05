#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

# First directory must be UNI which produces C/C++ header files

DIRS = Shared CMD REXX UtlAPI CPI PM MM SOM #Server

!include $(%ROOT)/mk/all.mk

all: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

clean: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

install: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
