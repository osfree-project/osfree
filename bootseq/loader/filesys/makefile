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
filesys      = $(p)fat$(e) $(p)ext2fs$(e) $(p)jfs$(e) $(p)iso9660$(e) $(p)reiserfs$(e) &
               $(p)minix$(e) $(p)ufs2$(e) $(p)vstafs$(e) $(p)ffs$(e) $(p)xfs$(e)

# filesys without $(p) and $(e):
p   =
e   =
fsys                 = $+$(filesys)$-

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
TARGETS      = $+$(filesys)$-

p            = $(PATH)
e            = .fsd
PROJ         = $+$(filesys)$- $(TARGETS)

files     = $(fsys)

p = fsys_
e = .c
spec_SRCS = $+$(filesys)$-

#
# See $(%ROOT)/mk/genrules.mk for details
#
gen_compile_rules_wrapper: $(MYDIR)$(file) .SYMBOLIC
!ifeq sh
 @$(MAKE) $(MAKEOPT) file=$[. ext=$(file:$[&=) e=.$$$$$$$$(O) defs="-d$[& -dSHIFT=0" gen_compile_rules
!else
 @$(MAKE) $(MAKEOPT) file=$[. ext=$(file:$[&=) e=.$$$$$$$$(SO) defs="-d$[& -dSHIFT=$$$$(SHIFT)" gen_compile_rules
!endif

$(PATH)fat.rel: $(PATH)fat.fsd $(PATH)fat.fss

$(PATH)ext2fs.rel: $(PATH)ext2fs.fsd $(PATH)ext2fs.fss

$(PATH)jfs.rel: $(PATH)jfs.fsd $(PATH)jfs.fss

$(PATH)ffs.rel: $(PATH)ffs.fsd $(PATH)ffs.fss

$(PATH)vstafs.rel: $(PATH)vstafs.fsd $(PATH)vstafs.fss

$(PATH)ufs2.rel: $(PATH)ufs2.fsd $(PATH)ufs2.fss

$(PATH)xfs.rel: $(PATH)xfs.fsd $(PATH)xfs.fss

$(PATH)reiserfs.rel: $(PATH)reiserfs.fsd $(PATH)reiserfs.fss

$(PATH)iso9660.rel: $(PATH)iso9660.fsd $(PATH)iso9660.fss

$(PATH)minix.rel: $(PATH)minix.fsd $(PATH)minix.fss


$(PATH)fat.fsd: $(PATH)fat.$(OUT)

$(PATH)ext2fs.fsd: $(PATH)ext2fs.$(OUT)

$(PATH)jfs.fsd: $(PATH)jfs.$(OUT)

$(PATH)ffs.fsd: $(PATH)ffs.$(OUT)

$(PATH)vstafs.fsd: $(PATH)vstafs.$(OUT)

$(PATH)ufs2.fsd: $(PATH)ufs2.$(OUT)

$(PATH)xfs.fsd: $(PATH)xfs.$(OUT)

$(PATH)reiserfs.fsd: $(PATH)reiserfs.$(OUT)

$(PATH)iso9660.fsd: $(PATH)iso9660.$(OUT)

$(PATH)minix.fsd: $(PATH)minix.$(OUT)


$(PATH)fat.fss: $(PATH)fat.$(SOUT)

$(PATH)ext2fs.fss: $(PATH)ext2fs.$(SOUT)

$(PATH)jfs.fss: $(PATH)jfs.$(SOUT)

$(PATH)ffs.fss: $(PATH)ffs.$(SOUT)

$(PATH)vstafs.fss: $(PATH)vstafs.$(SOUT)

$(PATH)ufs2.fss: $(PATH)ufs2.$(SOUT)

$(PATH)xfs.fss: $(PATH)xfs.$(SOUT)

$(PATH)reiserfs.fss: $(PATH)reiserfs.$(SOUT)

$(PATH)iso9660.fss: $(PATH)iso9660.$(SOUT)

$(PATH)minix.fss: $(PATH)minix.$(SOUT)
