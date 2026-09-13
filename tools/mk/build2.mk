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
!ifndef SOURCES
SOURCES=
!endif
!endif

!ifndef SOURCES

!include $(%ROOT)tools/mk/dirs.mk

MAKEOPT = -h PROJ=$(PROJ)

all install: .SYMBOLIC gen_sources
prepall:     .SYMBOLIC prep
depsall:     .SYMBOLIC deps

clean annotate: .symbolic
 @$(MAKE) $(MAKEOPT) gen_sources_files
 @$(MAKE) $(MAKEOPT) withsources=$(PATH)_sources.mk $^@

prep: .symbolic
 @$(MAKE) $(MAKEOPT) gen_sources_files
 @$(MAKE) $(MAKEOPT) withsources=$(PATH)_sources.mk TARGET=prepall subdirs

deps: .symbolic
 @$(MAKE) $(MAKEOPT) gen_sources_files
 @$(MAKE) $(MAKEOPT) withsources=$(PATH)_sources.mk TARGET=depsall subdirs

subdirs: .symbolic
!ifeq UNIX TRUE
 @for %d in ($(DIRS)) do @if [ -d $(MYDIR)%d ]; then @cd $(MYDIR)%d && @$(MAKE) -h $(MAKEOPT) $(TARGET) && cd ..; fi
!else
 @for %d in ($(DIRS)) do @if exist $(MYDIR)%d @$(CD) $(MYDIR)%d && @$(MAKE) -h $(MAKEOPT) $(TARGET)
!endif

!ifdef srcfile
_name2 = $(srcfile:$(MYDIR)=)
_name3 = $(_name2:$(ext)=)
!endif

add_source: .SYMBOLIC
!ifneq _name3 *
     @%append $(PATH)_sources.mk SOURCES += $(_name3)
!endif

gen_sources_files: .symbolic
 @$(REXX) mdhier.cmd $(PATH)
 @%create $(PATH)_sources.mk
 @for %f in ($(MYDIR)*.c) do @$(MAKE) -h $(MAKEOPT) srcfile=%f ext=.c add_source
 @for %f in ($(MYDIR)*.cpp) do @$(MAKE) -h $(MAKEOPT) srcfile=%f ext=.cpp add_source
 @for %f in ($(MYDIR)*.asm) do @$(MAKE) -h $(MAKEOPT) srcfile=%f ext=.asm add_source

gen_sources: .symbolic gen_sources_files
 @$(MAKE) -h $(MAKEOPT) withsources=$(PATH)_sources.mk


!else

# If we don't have SOURCES, NOLIBS, but DIRS, then just use old all.mk
!ifeq SOURCES
!ifndef LIBS
!ifdef DIRS
NO_DISPATCHER=1
!endif
!endif
!endif

!ifdef NO_DISPATCHER
!include $(%ROOT)tools/mk/all.mk
!else
!include $(%ROOT)tools/mk/build3.mk
!endif

!endif  

!endif  
