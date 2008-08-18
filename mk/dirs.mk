#
# set paths
#

!include $(%ROOT)/mk/site.mk

#drv =
#
#!ifneq %cdrive
#drv = $(%cdrive):
#!endif

MYDIR        = $(%cwd)$(SEP)
ROOT         = $(%ROOT)
RELDIR       = $(MYDIR:$(ROOT)=)
PATH         = $(RELDIR)

BLD  = $(ROOT)build$(SEP)

q = $(MYDIR:$(BLD)=)
x = build$(SEP)
#y = $(SEP)$(SEP)

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

# change slashes accordind to $(SEP)
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
