#
# A Makefile for UNIAPI
# (c) osFree project,
#

PROJ = uni2h
DESC = Unified Interfaces to C/C++ header files converter
INSTALL_ADD = 1

!include $(%ROOT)/mk/toolspas.mk

install_add: .SYMBOLIC
 @cd $(ROOT)$(SEP)UNI
 @$(MAKE) $(MAKEOPT) all
 @cd $(ROOT)$(SEP)tools$(SEP)uniapi
