#
# make macros
# for building tools
#

32_BITS  = 1
ADD_COPT = -d__WATCOM__ -d__OS2__ -i=. -i=..$(SEP)..$(SEP)include

!ifndef DEST
DEST    = tools
!endif

CLEANMASK = *.lnk *.wmp *.obj *.err *.log *.bak *.sym
ADD_LINKOPT = lib z,libmmap libpath $(FILESDIR)$(SEP)..$(SEP)lib

!include $(%ROOT)/mk/all.mk

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
