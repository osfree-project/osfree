# tools/mk/build.mk
# Universal target profile dispatcher for osFree build system
# Version: hierarchical class/subclass model
#
# ============================================================
# STAGE 1: Autodetections
# ============================================================

!ifndef __build_mk__
!define __build_mk__

# ------------------------------------------------------------
# Aliases
# ------------------------------------------------------------
!include $(%ROOT)tools/mk/build_aliases.mk


# ------------------------------------------------------------
# Autodetect project name from directory name
# ------------------------------------------------------------

!ifdef PROJ
#pass PROJ to child make to prevent another detection
MAKEOPT = PROJ=$(PROJ)
!include $(%ROOT)tools/mk/build2.mk
!else
# PROJ autodetection
!include $(%ROOT)tools/mk/build_proj.mk
!endif

!endif  
