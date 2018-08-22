#
# A main Makefile for osFree Command Line tools project
# (c) osFree project,
# valerius, 2006/10/30
#

32_BITS = 0
PROJ      = cmd
TRGT      = $(PROJ).com
DESC      = DOS command interpreter

DIRS = strings criter cmd suppl lib shell utils

!include $(%ROOT)/mk/all.mk
TARGETS=$(PROJ).com

$(PROJ).com: infores
  copy /b $(PATH)shell$(SEP)command.exe + $(PATH)infores + $(PATH)criter$(SEP)criter1 + $(PATH)criter$(SEP)criter + $(PATH)strings$(SEP)strings.dat $(PATH)command.com

infores:
 $(PATH)utils$(SEP)mkinfres.exe $(PATH)infores $(PATH)shell$(SEP)command.wmp $(PATH)shell$(SEP)command.exe
