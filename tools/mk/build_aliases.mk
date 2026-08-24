# tools/mk/build_aliases.mk
# Universal target profile dispatcher for osFree build system
# Version: hierarchical class/subclass model
#
# ============================================================
# Aliases definitions
# ============================================================

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
