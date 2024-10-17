#
# make macros for
# os2 16bit apps
#

!ifndef __appsos2v1_mk__
!define __appsos2v1_mk__

32_BITS = 0

ADD_COPT    = $(ADD_COPT) -d__OS2__ -bt=os2v1 -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)os21x -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)shared

!ifeq NOLIBS 1
ADD_LINKOPT +=        
!else
ADD_LINKOPT +=        lib $(BLD)lib$(SEP)cmd_shared16.lib, &
		      $(BLD)lib$(SEP)all_shared16.lib
!ifdef DEBUG
ADD_LINKOPT = DEBUG $(DEBUG) $(ADD_LINKOPT)
!endif

!endif

!ifndef DEST
DEST     = os2
!endif

!include $(%ROOT)/tools/mk/all.mk

ADD_COPT    +=        -i=$(%ROOT)$(SEP)include &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3

!ifdef DLL
TARGETS  = $(PATH)$(PROJ).dll # $(PATH)$(PROJ).sym
dllopts = dll
!ifdef DLLOPT
dllopts += $(DLLOPT)
!endif
!else
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym
dllopts =
!endif
RCOPT    = -bt=os2v1 -i=$(MYDIR) -i=$(PATH)

!ifdef RESOURCE
deps = $(RESOURCE)
!endif

!ifdef OBJS
#$(OBJS):: $(MYDIR)makefile
!endif

$(PATH)$(PROJ).lnk: $(deps) $(OBJS) $(MYDIR)makefile .always
 @%create $^@
 @%append $^@ SYSTEM os2 $(dllopts)
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
!ifdef STUB
 @%append $^@ OPTION STUB=$(STUB)
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
 @$(SAY) BIND     $^. $(LOG)
 $(verbose)$(BINDER) $^. $(LOG2)

!endif
