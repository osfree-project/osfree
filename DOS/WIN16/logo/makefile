#
# A Makefile for LOGO.LGO
# (c) osFree project,
# author, date
#

PROJ       = logo1
PROJ1      = logo
TRGT       = $(PROJ).com
DESC       = Windows Boot Logo screen
srcfiles   = $(p)logo$(e)
JWASM      = 1
COM        = 1
INSTALL_ADD = 1

TARGETS = $(PATH)$(PROJ1).lgo

!include $(%ROOT)/mk/appsdos.mk

.SUFFIXES: .lgo .com

.lgo: $(PATH)

install_add: .symbolic
 @$(SAY) INST     $(PROJ1).lgo $(LOG)
 @$(CP) $(PATH)$(PROJ).com $(PATH)$(PROJ1).lgo
 @$(CP) $(PATH)$(PROJ1).lgo $(DEST)
