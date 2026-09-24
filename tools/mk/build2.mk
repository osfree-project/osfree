# tools/mk/build2.mk
# Universal target profile dispatcher for osFree build system
# Version: hierarchical class/subclass model
#
# ============================================================
# STAGE 2: Sources detection
# ============================================================

!ifndef __build2_mk__
!define __build2_mk__

!ifdef PROJ

!ifdef withsources
!ifndef SOURCES
!include $(withsources)
!endif
MAKEOPT += withsources=$(withsources)
!ifndef SOURCES
SOURCES=
!endif
!endif

!ifndef SOURCES

!include $(%ROOT)tools/mk/dirs.mk

!ifeq UNIX TRUE
CD = cd
!else
CD = cd /d
!endif

TARGET = install

!ifeq %VERBOSE yes
verbose =
!else
verbose = @
!endif

all install: .SYMBOLIC gen_sources
depsall:     .SYMBOLIC deps

prepall: .symbolic
 @$(MAKE) $(MAKEOPT) gen_sources_files
 @$(MAKE) $(MAKEOPT) withsources=$(PATH)_sources.mk TARGET=prepall subdirs

clean annotate annotate-write: .symbolic
 @$(MAKE) $(MAKEOPT) gen_sources_files
 @$(MAKE) $(MAKEOPT) withsources=$(PATH)_sources.mk $^@

deps: .symbolic
 @$(MAKE) $(MAKEOPT) gen_sources_files
 @$(MAKE) $(MAKEOPT) withsources=$(PATH)_sources.mk TARGET=depsall subdirs

subdirs: .symbolic
!ifeq UNIX TRUE
 $(verbose)for %d in ($(DIRS)) do $(verbose)if [ -d $(MYDIR)%d ]; then $(verbose)cd $(MYDIR)%d && $(verbose)$(MAKE) $(__MAKEOPTS__) $(TARGET) PLATFORM=$(PLATFORM) && cd ..; fi
!else
 $(verbose)for %d in ($(DIRS)) do $(verbose)if exist $(MYDIR)%d $(verbose)$(CD) $(MYDIR)%d && $(verbose)$(MAKE) $(__MAKEOPTS__) $(TARGET) PLATFORM=$(PLATFORM)
!endif

!ifndef SRCDIR
SRCDIR = $(MYDIR)
!endif

!ifdef srcfile
_name2 = $(srcfile:$(SRCDIR)=)
_name3 = $(_name2:$(ext)=)
!endif

add_source: .SYMBOLIC
!ifneq _name3 *
     @%append $(PATH)_sources.mk SOURCES += $(_name3)
!endif

gen_sources_files: .symbolic
# @echo GS_CMD: $(MAKE) -h $(MAKEOPT) withsources=$(PATH)_sources.mk
 @$(REXX) mdhier.cmd $(PATH)
 @%create $(PATH)_sources.mk
 @%append $(PATH)_sources.mk !ifndef __$(PROJ)_sources_mk__
 @%append $(PATH)_sources.mk !define __$(PROJ)_sources_mk__
!ifneq TARGET_LANG pascal
 @for %f in ($(SRCDIR)*.c) do @$(MAKE) -h $(MAKEOPT) srcfile=%f ext=.c add_source
 @for %f in ($(SRCDIR)*.cpp) do @$(MAKE) -h $(MAKEOPT) srcfile=%f ext=.cpp add_source
 @for %f in ($(SRCDIR)*.asm) do @$(MAKE) -h $(MAKEOPT) srcfile=%f ext=.asm add_source
!endif
 @%append $(PATH)_sources.mk !endif

gen_sources: .symbolic gen_sources_files
 @$(MAKE) -h $(MAKEOPT) withsources=$(PATH)_sources.mk


!else

!ifdef PLATFORM
MAKEOPT += PLATFORM=$(PLATFORM)
!endif

# If we don't have SOURCES and LIBS, but DIRS, then just use old all.mk
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

!ifdef PORT_NAME
!include $(%ROOT)tools/mk/port.mk
!endif

!endif

!endif

!endif
