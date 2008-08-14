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

BLD  = $(ROOT)build

q = $(MYDIR:$(BLD)=)
x = build$(SEP)

!ifneq q $(MYDIR)
# we're starting make in build dir, not in src dir
PATH  = $(PATH:build=)
MYDIR = $(PATH:$(x)=)
!endif

PATH  = $(BLD)$(SEP)$(PATH)

TOOLDIR   = $(ROOT)$(SEP)tools$(SEP)

!ifneq ABSOLUTE_DEST
!ifneq DEST
# DEST is relative from $(FILESDIR)
DEST = $(FILESDIR)$(SEP)$(DEST)
!endif
!endif
