#
# make macros
# for building tools
#

!ifndef __tools_mk__
!define __tools_mk__

32_BITS  = 1

!ifndef DEST
DEST    = tools
!endif

!include $(%ROOT)/mk/all.mk
# Building tools for OS/2 on Linux
# system os2v2   
# -bt=os2v2 -d__OS2__

ADD_COPT = -i=. -i=..$(SEP)..$(SEP)include $(DEFINES) $(ADD_COPT)

# Building tools for Linux
# -dUNIX
!ifeq UNIX TRUE
OS       = UNIX
NO_DESCRIPTION = # option description "str" is not valid when build target is Linux.
ADD_LINKOPT    += system linux  debug dwarf all
ADD_COPT       += -i=$(%WATCOM)$(SEP)lh # -x
!endif

# The variable DEFINES is to append more compiler defines and switches from a singular makefile
# because add_copt is overwritten in this file.
CLEAN_ADD = *.c *.h

TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym

#$(PATH)$(PROJ)$(EXE_SUF): $(PATH)$(PROJ).lnk

$(PATH)$(PROJ).lnk: $(OBJS)
 @%create $^@
 @%append $^@ NAME $^*.exe
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

!endif
