#
# make macros
# for building tools
#

32_BITS  = 1

!ifndef DEST
DEST    = tools
!endif

CLEAN_ADD = *.oo2 *.ppo
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym

!include $(%ROOT)/mk/all.mk
