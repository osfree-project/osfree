#
# make macros for
# win16 apps
#

!ifndef DEST
DEST     = os2$(SEP)mdos$(SEP)winos2
!endif

!include $(%ROOT)/mk/all.mk

!ifdef DLL
TARGETS  = $(PATH)$(PROJ).dll # $(PATH)$(PROJ).sym
dllopt = dll
!else
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym
dllopt =
!endif
RCOPT    = -bt=windows

!ifdef RESOURCE
deps = $(RESOURCE)
!endif

$(PATH)$(PROJ).lnk: $(deps) .SYMBOLIC
 @%create $^@
 @%append $^@ SYSTEM windows $(dllopt)
 @%append $^@ NAME $^*
 @%append $^@ OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
!ifdef DEBUG
 @%append $^@ DEBUG $(DEBUG)
!endif
!ifdef STACKSIZE
 @%append $^@ OPTION ST=$(STACKSIZE)
!endif
!ifdef RESOURCE
 @%append $^@ OPTION RESOURCE=$(RESOURCE)
!endif
!ifdef IMPORTS
 @%append $^@ IMPORT $(IMPORTS)
!endif
!ifdef EXPORTS
 @%append $^@ EXPORT $(EXPORTS)
!endif
!ifdef OPTIONS
 @%append $^@ OPTION $(OPTIONS)
!endif
 @%append $^@ OPTION MAP=$^*.wmp
 $(ADDFILES_CMD)

!ifeq DLL 1
$(PATH)$(PROJ).dll: $(PATH)$(PROJ).lnk $(OBJS)
!else
$(PATH)$(PROJ).exe: $(PATH)$(PROJ).lnk $(OBJS)
!endif
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$[@ $(LOG)
