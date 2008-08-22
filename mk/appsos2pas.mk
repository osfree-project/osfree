#
# make macros
# for building tools
#

!ifndef __appsos2pas_mk__
!define __appsos2pas_mk__

32_BITS  = 1
CLEAN_ADD = *.oo2 *.ppo

!ifndef DEST
DEST    = os2
!endif

#CLEANMASK = *.lnk *.wmp *.obj *.err *.log *.bak *.sym
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym

!include $(%ROOT)/mk/all.mk

!endif
