# tools/mk/build.mk
# Universal target profile dispatcher for osFree build system
# Version: hierarchical class/subclass model
#
# ============================================================
# STAGE 1: Autodetections
# ============================================================

!ifndef __build_mk__
!define __build_mk__

!ifdef __LOADDLL__
! loaddll wcc wccd
! loaddll wccaxp wccdaxp
! loaddll wcc386 wccd386
! loaddll wpp wppdi86
! loaddll wppaxp wppdaxp
! loaddll wpp386 wppd386
! loaddll wlink wlinkd
! loaddll wlib wlibd
!endif

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
# ============================================================
# Autodetect TARGET_API depending on directory tree
# ============================================================

!ifndef TARGET_API

#!!!!!!!!!!!!! Тут проблема с DEST!!!!! Его надо определить до dirs.mk... Поэтому
# приходится дублировать макросы

!include $(%ROOT)/tools/mk/site.mk

!ifeq UNIX FALSE
CWD         = $(%cdrive):$(%cwd)$(SEP)
!else
CWD         = $(%cwd)$(SEP)
!endif

ROOT        = $(%ROOT)
BLD         = $(%ROOT)build$(SEP)

RD          = $(CWD:$(%ROOT)=)
RELDIR_PWD  = $(RD:build$(SEP)=)
RELDIR      = $(RELDIR_PWD:host$(SEP)$(%HOST)$(SEP)=)

# Add symbol to begining to mark start of path
TEST_STR = *$(RELDIR)

# Try to delete "*DOS\"
STRIPPED = $(TEST_STR:*DOS$(SEP)=)
#!message STRIPPED=$(STRIPPED)
# String changed - so we in DOS tree
!ifneq TEST_STR $(STRIPPED)

# Now check for WIN16 subtree
TEST_STR2 = *$(STRIPPED)
STRIPPED2 = $(TEST_STR2:*WIN16$(SEP)=)
#!message STRIPPED2=$(STRIPPED2)
!ifneq TEST_STR2 $(STRIPPED2)
#!message huh
TARGET_API=WIN
!else
TARGET_API=DOS
!endif

!endif

# Try same for OS/2. Try to delete "*OS2\"
STRIPPED = $(TEST_STR:*OS2$(SEP)=)

# String changed - so we in OS/2 tree
!ifneq TEST_STR $(STRIPPED)
TARGET_API=OS2
!endif

# Try same for tools. Try to delete "*tools\"
STRIPPED = $(TEST_STR:*tools$(SEP)=)

# String changed - so we in toolstree
!ifneq TEST_STR $(STRIPPED)
TARGET_API=HOST
!endif


!endif

!ifndef TARGET_API
!error TARGET_API must be set
!endif


!ifndef PLATFORM
!ifeq TARGET_API HOST
PLATFORM = host$(SEP)$(%HOST)$(SEP)
!endif
!endif

# ------------------------------------------------------------
# Goto Stage 2 
# ------------------------------------------------------------

!include $(%ROOT)tools/mk/build2.mk

!endif  
