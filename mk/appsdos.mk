#
#
#
#

!ifndef __appsdos_mk__
!define __appsdos_mk__

!include $(%ROOT)/mk/site.mk

ADD_COPT =            $(ADD_COPT) &
#                      -i=$(ROOT)$(SEP)build$(SEP)include &
#                      -i=$(ROOT)$(SEP)build$(SEP)include$(SEP)dos &
                      -bt=dos

!ifndef DEST
DEST     = os2$(SEP)mdos
!endif

!include $(%ROOT)/mk/all.mk

TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym

!ifeq COM 1
com = com
!else
com = 
!endif

$(PATH)$(PROJ).lnk: .SYMBOLIC
 @%create $^@
 @%append $^@ SYSTEM dos $(com)
 @%append $^@ NAME $^*
!ifdef ALIASES
 alias $(ALIASES)
!endif
!ifdef STACKSIZE
 @%append $^@ OPTION ST=$(STACKSIZE)
!endif
 @%append $^@ OPTION MAP=$^*.wmp
 $(ADDFILES_CMD)

$(PATH)$(PROJ).exe: $(PATH)$(PROJ).lnk $(OBJS)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$[@ $(LOG)

!endif
