#
#
#
#

!ifndef __appsdos_mk__
!define __appsdos_mk__

!include $(%ROOT)/mk/site.mk

!ifndef DEST
DEST     = os2$(SEP)mdos
!endif

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

!endif
