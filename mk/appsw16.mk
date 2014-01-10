#
# make macros for
# win16 apps
#

!ifndef __appsw16_mk__
!define __appsw16_mk__
#  -i=$(ROOT)$(SEP)DOS$(SEP)WIN16$(SEP)include 

ADD_COPT = -bt=windows -i=. -i=$(WATCOM)$(SEP)h$(SEP)win $(ADD_COPT)
ADD_LINKOPT = path $(WATCOM)$(SEP)lib286 &
  path $(WATCOM)$(SEP)lib286$(SEP)win &
  lib clibs.lib,windows.lib,shell.lib

#  file $(WATCOM)$(SEP)lib286$(SEP)dos$(SEP)cstart_t.obj

!ifndef DEST
DEST     = os2$(SEP)mdos$(SEP)winos2
!endif

!include $(%ROOT)/mk/all.mk

!ifdef DLL
TARGETS  = $(PATH)$(PROJ).dll
dllopt = dll
!else
TARGETS  = $(PATH)$(PROJ).exe
dllopt =
!endif

RC       = winrc
RCOPT    = -I $(MYDIR) -I $(PATH) -I . -I $(MYDIR)..$(SEP)..$(SEP)include -I $(MYDIR)..$(SEP)include

.SUFFIXES:
.SUFFIXES: .dll .exe .ico .obj .lnk .res .rc .h .c .cpp

.res: $(PATH)

.rc:  $(MYDIR)

.rc.res: .AUTODEPEND
 @$(SAY)    WINRES $[...
 @$(RC) $(RCOPT) -o $^@ $[@

!ifdef RESOURCE
deps = $(RESOURCE)
!endif

$(PATH)$(PROJ).lnk: $(deps) $(OBJS) .SYMBOLIC
 @%create $^@
 @%append $^@ SYSTEM windows $(dllopt)
 @%append $^@ NAME $^*
 @%append $^@ OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
#!ifdef DEBUG
# @%append $^@ DEBUG $(DEBUG)
#!endif
!ifdef HEAPSIZE
 @%append $^@ OPTION HEAP=$(HEAPSIZE)
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
!ifdef ALIASES
 alias $(ALIASES)
!endif
 @%append $^@ OPTION MAP=$^*.wmp
 $(ADDFILES_CMD)

!ifeq DLL 1
$(PATH)$(PROJ).dll: $(PATH)$(PROJ).lnk $(OBJS)
!else
$(PATH)$(PROJ).exe: $(PATH)$(PROJ).lnk $(OBJS)
!endif
 @$(SAY)     Linking $^@ $(LOG)
 @$(LINKER) $(LINKOPT) @$[@ $(LOG)

!endif
