#
# make macros
# for building tools
#

!ifndef __tools_mk__
!define __tools_mk__

32_BITS  = 1

PLATFORM = host$(SEP)$(%HOST)$(SEP)

!ifndef DEST
DEST    = $(PLATFORM)bin
!endif

!include $(%ROOT)/tools/mk/all.mk
# Building tools for OS/2 on Linux
# system os2v2   
# -bt=os2v2 -d__OS2__

ADD_COPT = $(ADD_COPT) -i=. -i=$(%ROOT)$(SEP)include $(DEFINES)

# Building tools for Linux
# -dUNIX
!ifeq UNIX TRUE
OS       = UNIX
NO_DESCRIPTION = # option description "str" is not valid when build target is Linux.
ADD_LINKOPT    += system linux
ADD_COPT       += -i=$(%WATCOM)$(SEP)lh # -x
!endif

!ifeq DLL 1
!ifeq %OS WIN32
ADD_LINKOPT    += system nt_dll
!endif
!ifeq %OS WIN64
ADD_LINKOPT    += system nt_dll
!endif
!endif

# The variable DEFINES is to append more compiler defines and switches from a singular makefile
# because add_copt is overwritten in this file.
CLEAN_ADD = *.c *.h

!ifeq DLL 1
TARGETS  = $(PATH)$(PROJ).dll # $(PATH)$(PROJ).sym
!else
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym
!endif

#$(PATH)$(PROJ)$(EXE_SUF): $(PATH)$(PROJ).lnk

!ifdef OBJS
#$(OBJS):: $(MYDIR)makefile
!endif

$(PATH)$(PROJ).lnk: $(OBJS) $(MYDIR)makefile .always
 @%create $^@
!ifeq DLL 1
 @%append $^@ NAME $^*.dll
!else
 @%append $^@ NAME $^*.exe
!endif
#!ifeq UNIX TRUE
# @%append $^@ debug dwarf all
#!endif
!ifdef STACKSIZE
 @%append $^@ OPTION ST=$(STACKSIZE)
!endif
!ifdef ALIAS
 @%append $^@ ALIAS $(ALIAS)
!endif
!ifndef UNIX
!ifndef NO_DESCRIPTION
 @%append $^@ OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
!endif
!endif
 @%append $^@ OPTION MAP=$^*.wmp
 $(ADDFILES_CMD)
!ifdef EXPORTS
 @%append $^@ EXPORT $(EXPORTS)
!endif

!endif
