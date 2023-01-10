#
# make macros
# for building tools
#

!ifndef __toolspas_mk__
!define __toolspas_mk__

32_BITS  = 1

PLATFORM = host$(SEP)$(%HOST)$(SEP)

!ifndef DEST
DEST    = $(PLATFORM)bin
!endif

CLEAN_ADD = *.oo2 *.ppo *.o

!include $(%ROOT)/tools/mk/all.mk

!ifeq DLL 1
TARGETS  = $(PATH)$(PROJ).dll # $(PATH)$(PROJ).sym
!else
TARGETS  = $(PATH)$(PROJ).exe # $(PATH)$(PROJ).sym
!endif

!ifdef RESOURCE
deps = $(RESOURCE)
!endif

$(TARGETS): $(deps) $(MYDIR)makefile

$(PATH)$(PROJ).lnk: $(OBJS) $(MYDIR)makefile .symbolic .always

!endif
