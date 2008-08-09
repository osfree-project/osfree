#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

#!include $(%ROOT)/build.conf
#!include $(%ROOT)/mk/site.mk

DIRS = minilibc filesys # preldr loader term kernels

!include $(%ROOT)/mk/loader.mk

all: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^& default

#default: .SYMBOLIC
# $(SAY) Making $(TARGET)... $(LOG)
# $(MAKE) $(MAKEOPT) TARGET=$(TARGET) subdirs
