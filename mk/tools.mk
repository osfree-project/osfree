#
# make macros
# for building tools
#

32_BITS  = 1
ADD_COPT = -d__WATCOM__ -d__OS2__ -i=. -i=..$(SEP)..$(SEP)include
#!ifndef DEST
DEST    = tools
#!endif
CLEANMASK = *.lnk *.wmp *.obj *.err *.log *.bak *.sym

!include $(%ROOT)/mk/all.mk

TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym

$(PATH)$(PROJ).lnk: .SYMBOLIC
 @%create $^@
 @%append $^@ NAME $^*
!ifdef STACKSIZE
 @%append $^@ OPTION ST=$(STACKSIZE)
!endif
 @%append $^@ OPTION MAP=$^*.wmp
 $(ADDFILES_CMD)

$(PATH)$(PROJ).exe: $(PATH)$(PROJ).lnk $(OBJS)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$[@ $(LOG)
