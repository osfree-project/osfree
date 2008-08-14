#
# make macros
# for building tools
#

32_BITS  = 1

!ifndef DEST
DEST    = tools
!endif

!include $(%ROOT)/mk/all.mk

CLEANMASK = *.lnk *.wmp *.obj *.err *.log *.bak *.sym
ADD_LINKOPT = $(ADD_LINKOPT) lib z,libmmap libpath $(FILESDIR)$(SEP)..$(SEP)build$(SEP)lib
ADD_COPT = -i=. -i=..$(SEP)..$(SEP)include
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym

$(PATH)$(PROJ).exe: $(PATH)$(PROJ).lnk

$(PATH)$(PROJ).lnk: $(OBJS) $(INCLUDES) .SYMBOLIC
 @%create $^@
 @%append $^@ NAME $^*
!ifdef STACKSIZE
 @%append $^@ OPTION ST=$(STACKSIZE)
!endif
 @%append $^@ OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^@ OPTION MAP=$^*.wmp
 $(ADDFILES_CMD)
