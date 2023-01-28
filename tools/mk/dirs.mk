#
# set paths
#

!ifndef __dirs_mk__
!define __dirs_mk__

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

MYDIR       = $(ROOT)$(RELDIR)
PATH        = $(BLD)$(PLATFORM)$(RELDIR)

TOOLDIR     = $(ROOT)tools$(SEP)
CONTRIB     = $(ROOT)contrib$(SEP)

!ifeq PORT_NAME subproj
PORT_BASE   = $(MYDIR)src$(SEP)
PORT_FLAG   = $(MYDIR)src.flg
!else
PORT_BASE   = $(CONTRIB)$(PORT_NAME)$(SEP)
PORT_FLAG   = $(CONTRIB)$(PORT_NAME).flg
!endif

!ifndef DEST
DEST = $(FILESDIR)os2
!else ifeq DEST none
# stay in build directory
DEST = $(PATH)
!else
# DEST is relative from $(FILESDIR)
DEST = $(FILESDIR)$(DEST)
!endif

!ifneq DEST1
# DEST is relative from $(FILESDIR)
DEST1 = $(FILESDIR)$(DEST1)
!endif

!endif
