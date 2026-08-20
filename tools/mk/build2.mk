# tools/mk/build2.mk
# Universal target profile dispatcher for osFree build system
# Version: hierarchical class/subclass model
#
# ============================================================
# STAGE 2: Sources detection
# ============================================================

!ifndef __build2_mk__
!define __build2_mk__

!ifdef withsources
!include $(withsources)
MAKEOPT = $(MAKEOPT) withsources=$(withsources)
!endif

!ifndef SOURCES

!include $(%ROOT)tools/mk/dirs.mk

all install: .SYMBOLIC gen_sources
prepall:     .SYMBOLIC prep

MAKEOPT = -h PROJ=$(PROJ)

!ifdef srcfile
_name2 = $(srcfile:$(MYDIR)=)
_name3 = $(_name2:$(ext)=)
!endif

add_source: .SYMBOLIC
!ifneq _name3 *
     @%append $(PATH)_sources.mk SOURCES += $(_name3)
!endif

gen_sources: .symbolic
  @$(REXX) mdhier.cmd $(PATH)
  @%create $(PATH)_sources.mk
  @for %f in ($(MYDIR)*.c) do @$(MAKE) -h $(MAKEOPT) srcfile=%f ext=.c add_source
  @for %f in ($(MYDIR)*.cpp) do @$(MAKE) -h $(MAKEOPT) srcfile=%f ext=.cpp add_source
  @for %f in ($(MYDIR)*.asm) do @$(MAKE) -h $(MAKEOPT) srcfile=%f ext=.asm add_source
  @$(MAKE) -h $(MAKEOPT) withsources=$(PATH)_sources.mk 


!else

!include $(%ROOT)tools/mk/build3.mk

!endif  

!endif  
