#
# A Makefile for LOGO.LGO
# (c) osFree project,
# author, date
#

PROJ        = logo
TRGT        = $(PROJ).com
DESC        = Windows Boot Logo screen
srcfiles    = $(p)logo$(e)
JWASM       = 1
COM         = 1
INSTALL_ADD = 1

!include $(%ROOT)/mk/appsdos.mk

.SUFFIXES: .lgo .com

.lgo: $(PATH)

install_add: .symbolic
 @$(SAY) INST     $(PROJ).lgo $(LOG)
 @$(CP) $(PATH)$(PROJ).com $(PATH)$(PROJ).lgo
 @$(CP) $(PATH)$(PROJ).lgo $(DEST)
