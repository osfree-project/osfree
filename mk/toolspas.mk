#
# make macros
# for building tools
#

!ifndef __toolspas_mk__
!define __toolspas_mk__

32_BITS  = 1

!ifndef DEST
DEST    = tools
!endif

CLEAN_ADD = *.oo2 *.ppo *.o

!include $(%ROOT)/mk/all.mk

TARGETS  = $(PATH)$(TRGT)

$(PATH)$(PROJ).lnk:
 @%null

!endif
