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
PORT_BASE   = $(CONTRIB)$(PORT_NAME)$(SEP)

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
