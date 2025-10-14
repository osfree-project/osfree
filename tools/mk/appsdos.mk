#
#
#
#

!ifndef __appsdos_mk__
!define __appsdos_mk__

!include $(%ROOT)/tools/mk/site.mk

ADD_COPT =            $(ADD_COPT) -bt=dos

!ifndef DEST
DEST     = os2$(SEP)mdos
!endif

!include $(%ROOT)/tools/mk/all.mk


!ifeq COM 1
comf = com
!else ifeq RAW 1
comf = com
!ifndef RAWEXT
RAWEXT = bin
!endif
!else
comf = 
!endif

!ifeq DLL 1
TARGETS  = $(PATH)$(PROJ).dll # $(PATH)$(PROJ).sym
!else ifeq COM 1
TARGETS  = $(PATH)$(PROJ).com # $(PATH)$(PROJ).sym
!else ifeq RAW 1
TARGETS  = $(PATH)$(PROJ).$(RAWEXT) # $(PATH)$(PROJ).sym
!else
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym
!endif

!ifdef OBJS
#$(OBJS):: $(MYDIR)makefile
!endif

$(PATH)$(PROJ).lnk: $(OBJS) $(MYDIR)makefile .always
 @%create $^@
 @%append $^@ FORMAT dos $(comf)
!ifeq RAW 1
 @%append $^@ NAME $^*.$(RAWEXT)
!else
 @%append $^@ NAME $^*
!endif
 @%append $^@ libpath %WATCOM%/lib286
 @%append $^@ libpath %WATCOM%/lib286/dos
!ifneq NOLIBS 1
!ifeq COM 1
 @%append $^@ libfile cstart_t.obj
!endif
!endif
!ifdef ALIASES
 alias $(ALIASES)
!endif
!ifdef STACKSIZE
 @%append $^@ OPTION ST=$(STACKSIZE)
!endif
 @%append $^@ OPTION MAP=$^*.wmp
 $(ADDFILES_CMD)

#$(PATH)$(TRGT): $(PATH)$(PROJ).lnk
# @$(SAY) LINK     $^. $(LOG)
# $(verbose)$(LINKER) $(LINKOPT) @$[@ $(LOG2)

$(PATH)$(PROJ).$(RAWEXT): $(PATH)$(PROJ).lnk
 @$(SAY) LINK     $^. $(LOG)
 @$(verbose)$(LINKER) $(LINKOPT) @$[@ $(LOG2)

!endif
