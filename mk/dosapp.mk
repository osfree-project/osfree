#
#
#
#

!include $(%ROOT)/mk/site.mk

DEST     = os2$(SEP)mdos

!include $(%ROOT)/mk/all.mk

TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym

$(PATH)$(PROJ).lnk: .SYMBOLIC
 @%create $^@
 @%append $^@ SYSTEM dos
 @%append $^@ NAME $^*
!ifdef STACKSIZE
 @%append $^@ OPTION ST=$(STACKSIZE)
!endif
 @%append $^@ OPTION MAP=$^*.wmp
 $(ADDFILES_CMD)

$(PATH)$(PROJ).exe: $(PATH)$(PROJ).lnk $(OBJS)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$[@ $(LOG)
