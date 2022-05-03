#
# A main Makefile for osFree Command Line tools project
# (c) osFree project,
# valerius, 2006/10/30
#

32_BITS = 0
PROJ      = command
TRGT      = $(PROJ).com
DESC      = DOS command interpreter

DIRS = strings criter cmd suppl lib shell utils
DEST = os2$(SEP)mdos

!include $(%ROOT)/mk/all.mk

#TARGETS=$(PATH)$(PROJ).com

$(PATH)$(PROJ).lnk:
 @%null

$(PATH)$(PROJ).com: $(PATH)shell$(SEP)$(PROJ).exe $(PATH)infores
!ifneq ENV LINUX
  @copy /b $(PATH)shell$(SEP)command.exe + $(PATH)infores + $(PATH)criter$(SEP)criter1$(SEP)criter1.exe + $(PATH)criter$(SEP)criter.exe + $(PATH)strings$(SEP)strings.dat $(PATH)command.com
  @copy $(PATH)command.com $(DEST)
!else
  @cat $(PATH)shell$(SEP)command.exe $(PATH)infores $(PATH)criter$(SEP)criter1$(SEP)criter1.exe $(PATH)criter$(SEP)criter.exe $(PATH)strings$(SEP)strings.dat >$(PATH)command.com
  @cp $(PATH)command.com $(DEST)
!endif

$(PATH)infores: $(PATH)shell$(SEP)command.wmp $(PATH)shell$(SEP)command.exe
 @$(PATH)utils$(SEP)mkinfres.exe $^@ $(PATH)shell$(SEP)command.wmp $(PATH)shell$(SEP)command.exe
