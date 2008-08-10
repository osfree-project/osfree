#
# set paths
#

!include $(%ROOT)/mk/site.mk

drv =

!ifneq %cdrive
drv = $(%cdrive):
!endif

MYDIR        = $(drv)$(%cwd)$(SEP)
ROOT         = $(%ROOT)
RELDIR       = $(%cwd:$(ROOT)=)$(SEP)
PATH         = $(RELDIR)

BLD  = $(ROOT)build$(SEP)

q = $(MYDIR:$(BLD)=)
x = build$(SEP)

!ifneq q $(MYDIR)
# we're starting make in build dir, not in src dir
PATH  = $(PATH:build=)
MYDIR = $(PATH:$(x)=)
!endif

PATH  = $(BLD)$(PATH)

TOOLDIR   = $(ROOT)$(SEP)tools$(SEP)

!ifndef ABSOLUTE_DEST
# DEST is relative from $(FILESDIR)
DEST = $(FILESDIR)$(SEP)$(DEST)
!endif

test: .SYMBOLIC
 $(SAY) $(PATH)
 $(SAY) $(MYDIR)
