# tools/mk/build.mk
# Universal target profile dispatcher for osFree build system
# Version: hierarchical class/subclass model
#
# ============================================================
# STAGE 1: Aliases and autodetections
# ============================================================

!ifndef __build_mk__
!define __build_mk__

# ------------------------------------------------------------
# DESC=DESCRIPTION alias
# ------------------------------------------------------------

!ifdef DESCRIPTION
DESC=$(DESCRIPTION)
!endif

# ------------------------------------------------------------
# PROJ=PROJECT alias
# ------------------------------------------------------------

!ifdef PROJECT
!ifndef PROJ
PROJ=$(PROJECT)
!endif
!endif

# ------------------------------------------------------------
# LIBS=LIBRARIES alias
# ------------------------------------------------------------

!ifdef LIBRARIES
LIBS=$(LIBRARIES)
!endif

# ------------------------------------------------------------
# DIRS=DIRECTORIES alias
# ------------------------------------------------------------

!ifdef DIRECTORIES
DIRS=$(DIRECTORIES)
!endif

# ------------------------------------------------------------
# DEST=DESTINATION alias
# ------------------------------------------------------------

!ifdef DESTINATION
DEST=$(DESTINATION)
!endif

# ------------------------------------------------------------
# Autodetect project name from directory name
# ------------------------------------------------------------

!ifndef PROJ
!include $(%ROOT)tools/mk/dirs.mk

all: gen_proj_name
install: gen_proj_name

MAKEOPT = -h

dir2=$+ $(CWD) $-
dir3=$(dir2:$(CWD)=)
dir4=$(dir3:$(SEP)=)

gen_proj_name: .SYMBOLIC
 #generate project name
 @$(REXX) mdhier.cmd $(PATH)
 @cd ..
 @if not exist $(PATH)$(dir4)$(SEP)_proj.mk @%append $(PATH)$(dir4)$(SEP)_proj.mk PROJ=$(dir4)
 @cd $(dir4) && @$(MAKE) $(MAKEOPT) PROJ=$(dir4)
!else

!include $(%ROOT)tools/mk/build2.mk

!endif

!endif  
