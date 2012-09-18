#
# make macros
# for building tools
#

!ifndef __tools_mk__
!define __tools_mk__

32_BITS  = 1

!ifndef DEST
DEST    = tools
!endif

!include $(%ROOT)/mk/all.mk

CLEAN_ADD = *.c *.h
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym

$(PATH)$(PROJ).exe: $(PATH)$(PROJ).lnk

$(PATH)$(PROJ).lnk: $(OBJS) $(INCLUDES) .SYMBOLIC
 @%create $^@
 @%append $^@ NAME $^*.exe
!ifdef STACKSIZE
 @%append $^@ OPTION ST=$(STACKSIZE)
!endif
!ifndef UNIX
 @%append $^@ OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
!endif
 @%append $^@ OPTION MAP=$^*.wmp
 $(ADDFILES_CMD)

!endif
