#
# set paths
#

!ifndef __dirs_mk__
!define __dirs_mk__

!include $(%ROOT)/mk/site.mk

MYDIR        = $(%cwd)$(SEP)
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

PATH  = $(BLD)$(PATH)

# change two slashes into one
PATH  = $(PATH:\\=\)
PATH  = $(PATH://=\)
PATH  = $(PATH:\/=\)
PATH  = $(PATH:/\=\)

MYDIR = $(MYDIR:\\=\)
MYDIR = $(MYDIR://=\)
MYDIR = $(MYDIR:\/=\)
MYDIR = $(MYDIR:/\=\)

# change slashes according to $(SEP)
!ifeq SEP \
PATH  = $(PATH:/=\)
MYDIR = $(MYDIR:/=\)
!else
PATH  = $(PATH:\=/)
MYDIR = $(MYDIR:\=/)
!endif

TOOLDIR   = $(ROOT)$(SEP)tools$(SEP)

!ifneq ABSOLUTE_DEST
!ifneq DEST
# DEST is relative from $(FILESDIR)
DEST = $(FILESDIR)$(SEP)$(DEST)
!endif
!endif

!endif
