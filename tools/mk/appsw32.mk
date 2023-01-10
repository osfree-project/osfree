#
#
#

!ifndef __appsw32_mk__
!define __appsw32_mk__

32_BITS = 1
PLATFORM = nt
CLEAN_ADD = *.inf *.cmd *.msg *.pl *.ru *.rsf *.c *.h
ADD_COPT =            -d__WIN32__ -d__WINNT__ $(ADD_COPT) &
                      -i=$(%WATCOM)$(SEP)h$(SEP)nt -i=. -i=.. -i=win32 &
                      -i=$(%ROOT)$(SEP)build$(SEP)include &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)os2 &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3 &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)GDlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)zlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)lpng &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)jpeglib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)libtiff &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)gbm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pdcurses &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)glib &
                      -bt=nt
ADD_LINKOPT =         $(ADD_LINKOPT) OPTION REDEFSOK lib all_shared.lib,cmd_shared.lib

!ifndef DEST
DEST    = nt
!endif

!include $(%ROOT)/tools/mk/all.mk

!ifeq DLL 1
TARGETS  = $(PATH)$(PROJ).dll # $(PATH)$(PROJ).sym
dllopts = dll
!ifdef DLLOPT
dllopts = $(dllopts) $(DLLOPT)
!endif
!else
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym
dllopts =
!endif
RCOPT    = -bt=nt $(RCOPT)

!ifdef RESOURCE
deps = $(RESOURCE)
!endif

!ifdef OBJS
#$(OBJS):: $(MYDIR)makefile
!endif

$(PATH)$(PROJ).lnk: $(deps) $(OBJS) $(MYDIR)makefile .always
 @%create $^@
 @%append $^@ SYSTEM win32 $(dllopts)
 @%append $^@ NAME $^*
 @%append $^@ ru console
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
!ifdef ALIASES
 @%append $^@ ALIAS  $(ALIASES)
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

!endif
