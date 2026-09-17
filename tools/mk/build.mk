# tools/mk/build.mk
# Universal target profile dispatcher for osFree build system
# Version: hierarchical class/subclass model
#
# ============================================================
# STAGE 1: Autodetections
# ============================================================

!ifndef __build_mk__
!define __build_mk__

#!message $(%CWD)
#!message $(SOURCES)
#!message $(__MAKEFILES__)

# ------------------------------------------------------------
# Aliases
# ------------------------------------------------------------
!include $(%ROOT)tools/mk/build_aliases.mk

#-------------------------------------------------------------
# Undefine empty macros passed via command line
#-------------------------------------------------------------

!ifdef PLATFORM
#!message build.mk: PLATFORM is [$(PLATFORM)], undefining
!ifeq PLATFORM
#!message Undefining PLATFORM
!undef PLATFORM
!endif
!ifndef PLATFORM
#!message build.mk: PLATFORM not defined now
!else
#!message build.mk: PLATFORM still defined
!endif
!endif

!ifdef PROJ
#!message build.mk: PROJ is [$(PROJ)]
!ifeq PROJ
#!message Undefining PROJ
!undef PROJ
!endif
!ifndef PROJ
#!message build.mk: PROJ not defined now
!else
#!message build.mk: PROJ still defined
!endif
!endif

MAKEOPT = $(__MAKEOPTS__)

!ifdef PROJ
MAKEOPT += PROJ=$(PROJ)
!endif

!ifdef PLATFORM
MAKEOPT += PLATFORM=$(PLATFORM)
!endif

# ------------------------------------------------------------
# Autodetect project name from directory name
# ------------------------------------------------------------

!include $(%ROOT)tools/mk/build_proj.mk

# ------------------------------------------------------------
# Goto Stage 2 
# ------------------------------------------------------------

!include $(%ROOT)tools/mk/build2.mk

!endif  
