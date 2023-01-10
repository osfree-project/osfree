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

!include $(%ROOT)/tools/mk/all.mk

!ifdef DLL
TARGETS  = $(PATH)$(PROJ).dll # $(PATH)$(PROJ).sym
dllopt = dll
!else
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym
dllopt =
!endif

RC       = winrc.exe
RCOPT    = -I$(MYDIR) -I$(PATH) -I. -I$(MYDIR)..$(SEP)..$(SEP)include -I$(MYDIR)..$(SEP)include
# RCOPT    = -I $(MYDIR);$(PATH);.;$(MYDIR)..$(SEP)..$(SEP)include;$(MYDIR)..$(SEP)include

#.res: $(PATH)

#.rc:  $(MYDIR)

.rc.res: .AUTODEPEND
 @$(SAY) WINRES   $^. $(LOG)
 @$(RC) $(RCOPT) -o $^@ $[@ $(LOG2)

!ifdef RESOURCE
deps = $(RESOURCE)
!endif

!ifdef OBJS
#$(OBJS):: $(MYDIR)makefile
!endif

$(PATH)$(PROJ).lnk: $(deps) $(OBJS) $(MYDIR)makefile .always
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
$(PATH)$(PROJ).dll: $(PATH)$(PROJ).lnk
!else
$(PATH)$(PROJ).exe: $(PATH)$(PROJ).lnk
!endif
 @$(SAY) LINK     $^. $(LOG)
 $(verbose)$(LINKER) $(LINKOPT) @$[@ $(LOG2)

!endif
