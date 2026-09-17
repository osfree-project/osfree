# tools/mk/build_aliases.mk
# Universal target profile dispatcher for osFree build system
# Version: hierarchical class/subclass model
#
# ============================================================
# Aliases definitions
# ============================================================
#
# New build system dispatche operates with full names, but 
# old build system uses short names. We want  migrate to
# full names to reduce conflict possibilities
# After full migration to new build system we will replace
# this file to support short form in component makefile, but
# full name in build system.

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
# DEPS=DEPENDENCIES alias
# ------------------------------------------------------------

!ifdef DEPENDENCIES
DEPS=$(DEPENDENCIES)
!endif
