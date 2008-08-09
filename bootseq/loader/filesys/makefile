#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

#
# $+ switches on and $- switches off
# immediate macros substitution
#

#
# filesystems list with path (p) and extension (e) placeholder
#
blackboxes           = $(p)fat$(e) $(p)ext2fs$(e) $(p)jfs$(e) $(p)iso9660$(e) $(p)reiserfs$(e) &
                       $(p)minix$(e) $(p)ufs2$(e) $(p)vstafs$(e) $(p)ffs$(e) $(p)xfs$(e)

# filesys without $(p) and $(e):
p   =
e   =
bbx                  = $+$(blackboxes)$-

common_SRCS          = dummy.asm start.asm fsys.c
common_OBJS          = $(p)start$(e)  $(p)fsys$(e) $(p)dummy$(e)
OBJS                 = $(common_OBJS) $(p)fsys_$(basename)$(e)

DIRS         =
DIR          = filesys
RELDIR       = bootseq$(SEP)loader$(SEP)$(DIR)$(SEP)
MYDIR        = $(%ROOT)$(SEP)$(RELDIR)
PATH         = $(RELDIR)

32_BITS      = 1       # Use 32-bit C compiler
DEFINES      = -dNO_DECOMPRESSION # -dSTAGE1_5 -dNO_BLOCK_FILES -dOS2 -d__WATCOM__
ADD_COPT     = -s $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=$(MYDIR)include -i=$(MYDIR)..$(SEP)include -i=. -i=..
ADD_ASMOPT   = $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=$(MYDIR)include  -i=$(MYDIR)..$(SEP)include -i=. -i=..


!include $(%ROOT)/mk/loader.mk

# note that PATH is changed in loader.mk,
# so it must be included before this place:
p            = $(PATH)
e            = .rel
TARGETS      = $+$(blackboxes)$-

p            = $(PATH)
e            = .fsd
PROJ         = $+$(blackboxes)$- $(TARGETS)

files     = $(bbx)

p = fsys_
e = .c
spec_SRCS = $+$(blackboxes)$-


F = 1

!ifeq sh
defs=-d$(basename) -dSHIFT=0
!else
defs=-d$(basename) -dSHIFT=$$(SHIFT)
!endif

