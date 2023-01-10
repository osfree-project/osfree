#
#
#

!ifndef __appslnx_mk__
!define __appslnx_mk__

32_BITS = 1
PLATFORM = linux
CLEAN_ADD = *.inf *.cmd *.msg *.pl *.ru *.rsf *.c *.h
EXE_SUFFIX=l
ADD_COPT =            -d__LINUX__ $(ADD_COPT) &
                      -i=$(%WATCOM)$(SEP)lh -i=. -i=.. -i=Linux &
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
                      -bt=linux
ADD_LINKOPT =         $(ADD_LINKOPT) OPTION REDEFSOK lib all_shared.lib,cmd_shared.lib libpath $(ROOT)build$(SEP)lib

!ifndef DEST
DEST    = linux
!endif


!include $(%ROOT)/tools/mk/all.mk

!ifeq DLL 1
TARGETS  = $(PATH)$(PROJ).so # $(PATH)$(PROJ).sym
dllopts = dll
!ifdef DLLOPT
dllopts = $(dllopts) $(DLLOPT)
!endif
!else
TARGETS  = $(PATH)$(PROJ)$(EXE_SUFFIX) # $(PATH)$(PROJ).sym
dllopts =
!endif
#RCOPT    = -bt=linux $(RCOPT)

!ifdef RESOURCE
deps = $(RESOURCE)
!endif

!ifdef OBJS
#$(OBJS):: $(MYDIR)makefile
!endif

$(PATH)$(PROJ).lnk: $(deps) $(OBJS) $(MYDIR)makefile .always
 @%create $^@
 #@%append $^@ system linux dll
 @%append $^@ format elf $(dllopts)
!ifndef DLL
 @%append $^@ NAME $^*.exe
!else
 @%append $^@ NAME $^*.dll
!endif
 #@%append $^@ OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
!ifdef STACKSIZE
 @%append $^@ OPTION ST=$(STACKSIZE)
!endif
#!ifdef RESOURCE
# @%append $^@ OPTION RESOURCE=$(RESOURCE)
#!endif
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
#!ifdef STUB
# @%append $^@ OPTION STUB=$(STUB)
#!endif
 #@%append $^@ OPTION noextension
 @%append $^@ OPTION caseexact
 @%append $^@ OPTION MAP=$^*.wmp
 $(ADDFILES_CMD)

!ifeq DLL 1
$(PATH)$(PROJ).so: $(PATH)$(PROJ).lnk
!else
$(PATH)$(PROJ)$(EXE_SUFFIX): $(PATH)$(PROJ).lnk
!endif
 @$(SAY) LINK      $^. $(LOG)
 $(verbose)$(LINKER) $(LINKOPT) @$[@ $(LOG2)

!endif
