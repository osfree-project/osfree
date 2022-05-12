#
# A Makefile for LOGO.LGO
# (c) osFree project,
# author, date
#

PROJ       = logo1
PROJ1      = logo
TRGT       = $(PROJ1).lgo
DESC       = Windows Boot Logo screen
srcfiles   = $(p)logo$(e)
JWASM      = 1
COM        = 1

!include $(%ROOT)/mk/appsdos.mk

TARGETS = $(PATH)$(PROJ1).lgo

$(PATH)$(PROJ1).lgo: $(PATH)$(PROJ).com
 $(RN) $]@ $[@
