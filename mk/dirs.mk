#
# set paths
#

!ifndef __dirs_mk__
!define __dirs_mk__

!include $(%ROOT)/mk/site.mk

# SEP does not have a valid value here. Set it if it's undefined. 
!ifdef %SEP
SEP = $(%SEP)
!endif
#                     
MYDIR        = $(%cwd)$(SEP)
ROOT         = $(%ROOT)
RELDIR       = $(MYDIR:$(ROOT)=)
##RELDIR       = $(ROOT:$(MYDIR)=)
PATH         = $(RELDIR)

# Files from which HDD image is built
FILESDIR   = $(ROOT)$(SEP)bin

BLD  = $(ROOT)$(SEP)build$(SEP)

q = $(MYDIR:$(BLD)=)
x = build

# !ifneq q $(MYDIR)
!ifneq q $(MYDIR)
# we're starting make in build dir, not in src dir
PATH  = $(PATH:$(SEP)build=)
##PATH  = $(PATH:build=)
MYDIR = $(PATH:$(x)=)
!endif

#mydir  <--  $path:$x
#path   <--  $reldir
#x      <--  build
#reldir <--  $mydir:$root
#
#mydir  <--  $path:$x
#            path   <--  $reldir
#            x      <--  build
#                        reldir <--  $mydir:$root
#                                        $mydir <---- Start at beginning
PATH  = $(BLD)$(SEP)$(PATH)

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

!ifndef DEST
DEST = $(FILESDIR)$(SEP)os2
!else
!ifeq DEST none
# stay in build directory
DEST = $(PATH)
!else
# DEST is relative from $(FILESDIR)
DEST = $(FILESDIR)$(SEP)$(DEST)
!endif
!endif

!ifneq DEST1
# DEST is relative from $(FILESDIR)
DEST1 = $(FILESDIR)$(SEP)$(DEST1)
!endif

!endif
