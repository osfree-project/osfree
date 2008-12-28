#
# make macros for
# win16 apps
#

!ifndef __appsos2v1_mk__
!define __appsos2v1_mk__

32_BITS = 0

ADD_COPT = -bt=os2 -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)os21x $(ADD_COPT)
ADD_LINKOPT = lib clibs.lib,os2.lib

!ifndef DEST
DEST     = os2
!endif

!include $(%ROOT)/mk/all.mk

!ifdef DLL
TARGETS  = $(PATH)$(PROJ).dll # $(PATH)$(PROJ).sym
dllopt = dll
!else
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym
dllopt =
!endif
RCOPT    = -bt=os2 -i=$(MYDIR) -i=$(PATH)

!ifdef RESOURCE
deps = $(RESOURCE)
!endif

$(PATH)$(PROJ).lnk: $(deps) .SYMBOLIC
 @%create $^@
 @%append $^@ SYSTEM os2 $(dllopt)
 @%append $^@ NAME $^*
 @%append $^@ OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
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
!ifdef DEBUG
 @%append $^@ DEBUG $(DEBUG)
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

!endif
