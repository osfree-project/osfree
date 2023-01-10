#
# set paths
#

!ifndef __dirs_mk__
!define __dirs_mk__

!include $(%ROOT)/tools/mk/site.mk

!ifeq UNIX FALSE
MYDIR        = $(%cdrive):$(%cwd)$(SEP)
!else
MYDIR        = $(%cwd)$(SEP)
!endif

ROOT         = $(%ROOT)

RELDIR       = $(MYDIR:$(ROOT)=)

PATH         = $(RELDIR)

BLD  = $(ROOT)build$(SEP)

q = $(MYDIR:$(BLD)=)
x = build

!ifneq q $(MYDIR)
# we're starting make in build dir, not in src dir
PATH  = $(PATH:build=)
MYDIR = $(PATH:$(x)=)
!endif

PATH  = $(BLD)$(PLATFORM)$(PATH)

# change two slashes into one
PATH  = $(PATH://=/)
PATH  = $(PATH:/\=/)
PATH  = $(PATH:\/=/)
PATH  = $(PATH:\\=/)

MYDIR = $(MYDIR://=/)
MYDIR = $(MYDIR:/\=/)
MYDIR = $(MYDIR:\/=/)
MYDIR = $(MYDIR:\\=/)

# change slashes according to $(SEP)
!ifneq SEP /
PATH  = $(PATH:/=\)
MYDIR = $(MYDIR:/=\)
!endif

TOOLDIR   = $(ROOT)$(SEP)tools$(SEP)

!ifndef DEST
DEST = $(FILESDIR)$(SEP)os2
!else ifeq DEST none
# stay in build directory
DEST = $(PATH)
!else
# DEST is relative from $(FILESDIR)
DEST = $(FILESDIR)$(SEP)$(DEST)
!endif

!ifneq DEST1
# DEST is relative from $(FILESDIR)
DEST1 = $(FILESDIR)$(SEP)$(DEST1)
!endif

!endif
