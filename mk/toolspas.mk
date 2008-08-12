#
# make macros
# for building tools
#

32_BITS  = 1

!ifndef DEST
DEST    = tools
!endif

CLEANMASK = *.lnk *.wmp *.obj *.err *.log *.bak *.sym
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym

!include $(%ROOT)/mk/all.mk
