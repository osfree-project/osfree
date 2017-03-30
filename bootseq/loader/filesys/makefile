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
                       $(p)minix$(e) $(p)ufs2$(e) $(p)vstafs$(e) $(p)ffs$(e) $(p)xfs$(e) $(p)ntfs$(e) &
                       $(p)ntfs_nocomp$(e) $(p)hpfs$(e)

# filesys without $(p) and $(e):
p                    =
e                    =
bbx                  = $+$(blackboxes)$-

ADD_COPT             = -s -j # -dSTAGE1_5

!ifeq T ntfs_nocomp
ALIASES              = ntfs_nocomp_mount_=ntfs_mount_, &
                       ntfs_nocomp_dir_=ntfs_dir_, &
                       ntfs_nocomp_read_=ntfs_read_
!endif

common_SRCS          = start.asm fsys.c dummy.asm
common_OBJS          = $(p)start$(e) $(p)fsys$(e) $(p)dummy$(e)
OBJS                 = $(common_OBJS) $(p)fsys_$(basename)$(e)

MOD_BASE             = EXT_BUF_BASE
DEST                 = boot$(SEP)loader$(SEP)fsd

!include $(%ROOT)/mk/loader.mk

# note that PATH is changed in loader.mk,
# so it must be included before this place:
p            = $(PATH)
e            = .rel
rels         = $+$(blackboxes)$-

p            = $(PATH)
e            = .mdl
TARGETS      = $+$(blackboxes)$- $(rels)

files     = $(bbx)

p = fsys_
e = .c
spec_SRCS = $+$(blackboxes)$-

F = 1

!ifeq sh sh_
defs=-d$(basename) -dSHIFT=$$(SHIFT)
!else
defs=-d$(basename) -dSHIFT=0
!endif

$(PATH)start.$(O): $(MYDIR)start.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -d$(basename) -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)start.$(SO): $(MYDIR)start.asm
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -d$(basename) -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)fsys.$(O): $(MYDIR)fsys.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=0 -dSTAGE1_5 $(COPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)fsys.$(SO): $(MYDIR)fsys.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=$(SHIFT) -dSTAGE1_5 $(COPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)fsys_ntfs_nocomp.$(O): $(MYDIR)fsys_ntfs1.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dfsys_ntfs -dSHIFT=0 -dSTAGE1_5 -dNO_NON_RESIDENT_ATTRIBUTE_LIST -dNO_ALTERNATE_DATASTREAM -dNO_ATTRIBUTE_LIST $(COPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)fsys_ntfs_nocomp.$(SO): $(MYDIR)fsys_ntfs1.c
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dfsys_ntfs -dSHIFT=$(SHIFT) -dSTAGE1_5 -dNO_NON_RESIDENT_ATTRIBUTE_LIST -dNO_ALTERNATE_DATASTREAM -dNO_ATTRIBUTE_LIST $(COPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)
