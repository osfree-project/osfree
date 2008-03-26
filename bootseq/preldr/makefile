#
# Makefile for microFSD's and stage0.
# 07/10/04 valerius
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS      = 1       # Use 32-bit C compiler
DEFINES      = -dNO_DECOMPRESSION # -dSTAGE1_5 -dNO_BLOCK_FILES -dOS2 -d__WATCOM__
ADD_COPT     = -s $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=include -i=. -i=..
ADD_ASMOPT   = $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=include -i=. -i=..

DIR          = preldr
MYDIR        = $(ROOT)$(SEP)bootseq$(SEP)$(DIR)$(SEP)
PATH         = $(MYDIR)

!include $(%ROOT)/mk/bootseq.mk

CLEANMASK    = $(CLEANMASK) *.sob *.lob

RIP          = $(REXX) ripzeroes.cmd
GENREL       = $(REXX) genrel.cmd

SO           = sob
LO           = lob

OUT          = bin
SOUT         = sbi
LOUT         = lbi

.SUFFIXES:
.SUFFIXES: .rel .fsd .fss .$(OUT) .$(LOUT) .$(SOUT) .$(LO) .$(SO) .$(O) .c .asm .h .inc

FS_OBJS             = start.$(O)  fsys.$(O)  dummy.$(O)
FS_SH_OBJS          = start.$(SO) fsys.$(SO) dummy.$(SO)

FAT_OBJS            = $(FS_OBJS) fsys_fat.$(O)
EXT2FS_OBJS         = $(FS_OBJS) fsys_ext2fs.$(O)
JFS_OBJS            = $(FS_OBJS) fsys_jfs.$(O)
ISO9660_OBJS        = $(FS_OBJS) fsys_iso9660.$(O)
REISERFS_OBJS       = $(FS_OBJS) fsys_reiserfs.$(O)
MINIX_OBJS          = $(FS_OBJS) fsys_minix.$(O)
UFS2_OBJS           = $(FS_OBJS) fsys_ufs2.$(O)
VSTAFS_OBJS         = $(FS_OBJS) fsys_vstafs.$(O)
FFS_OBJS            = $(FS_OBJS) fsys_ffs.$(O)
XFS_OBJS            = $(FS_OBJS) fsys_xfs.$(O)

FAT_SH_OBJS         = $(FS_SH_OBJS) fsys_fat.$(SO)
EXT2FS_SH_OBJS      = $(FS_SH_OBJS) fsys_ext2fs.$(SO)
JFS_SH_OBJS         = $(FS_SH_OBJS) fsys_jfs.$(SO)
ISO9660_SH_OBJS     = $(FS_SH_OBJS) fsys_iso9660.$(SO)
REISERFS_SH_OBJS    = $(FS_SH_OBJS) fsys_reiserfs.$(SO)
MINIX_SH_OBJS       = $(FS_SH_OBJS) fsys_minix.$(SO)
UFS2_SH_OBJS        = $(FS_SH_OBJS) fsys_ufs2.$(SO)
VSTAFS_SH_OBJS      = $(FS_SH_OBJS) fsys_vstafs.$(SO)
FFS_SH_OBJS         = $(FS_SH_OBJS) fsys_ffs.$(SO)
XFS_SH_OBJS         = $(FS_SH_OBJS) fsys_xfs.$(SO)

STAGE0_OBJS      = segord.$(O) init.$(O) modesw.$(O) asmcode.$(O) apm.$(O) biosdisk.$(O) filesys.$(O) &
                   video.$(O) common.$(O) stage0_fs.$(O) func.$(O) minilibc.$(O) bios.$(O) end.$(O)

STAGE0_SH_OBJS   = segord.$(SO) init.$(SO) modesw.$(SO) asmcode.$(SO) apm.$(SO) biosdisk.$(SO) filesys.$(SO) &
                   video.$(SO) common.$(SO) stage0_fs.$(SO) func.$(SO) minilibc.$(SO) bios.$(SO) end.$(SO)

STAGE0_LT_OBJS   = segord.$(LO) init.$(LO) modesw.$(LO) asmcode.$(LO) apm.$(LO) biosdisk.$(LO) filesys.$(LO) &
                   video.$(LO) common.$(LO) stage0_fs.$(LO) func.$(LO) minilibc.$(LO) bios.$(LO) end.$(LO)

LDR_OBJS         = ldrstart.$(O) loader.$(O) wrap.$(O) commands.$(O) &
                   boot.$(O) minilibc.$(O) cmdline.$(O) # video.$(O)

TARGETS          = fat.fsd ext2fs.fsd jfs.fsd iso9660.fsd reiserfs.fsd &
                   minix.fsd ufs2.fsd vstafs.fsd ffs.fsd xfs.fsd preldr0 preldr0_lite freeldr &
                   fat.fss ext2fs.fss jfs.fss iso9660.fss reiserfs.fss &
                   minix.fss ufs2.fss vstafs.fss ffs.fss xfs.fss preldr0s &
                   fat.rel ext2fs.rel jfs.rel iso9660.rel reiserfs.rel &
                   minix.rel ufs2.rel vstafs.rel ffs.rel xfs.rel preldr0.rel

SHIFT = 0x100

all: fsd.h $(TARGETS) .SYMBOLIC

.fsd.rel
 $(GENREL) $^&.fsd $^&.fss $(SHIFT) >$^@
 $(DC) $^&.fss

preldr0.rel: preldr0
 $(GENREL) $^& $^&s $(SHIFT) >$^@
 $(DC) $^&s

.$(OUT).fsd:
 $(DC) $^@
 $(RIP) $< EXT_BUF_BASE fsd.inc >$^@
 $(DC) $<

.$(SOUT).fss:
 $(DC) $^@
 $(RIP) $< EXT_BUF_BASE fsd.inc $(SHIFT) >$^@
 $(DC) $<

preldr0: stage0.$(OUT)
 $(DC) $^@
 $(RIP) $< STAGE0_BASE fsd.inc >$^@
 $(DC) $<

preldr0_lite: stage0.$(LOUT)
 $(DC) $^@
 $(RIP) $< STAGE0_BASE fsd.inc >$^@
 $(DC) $<

preldr0s: stage0.$(SOUT)
 $(DC) $^@
 $(RIP) $< STAGE0_BASE fsd.inc $(SHIFT) >$^@
 $(DC) $<

freeldr: freeldr.$(OUT)
 $(DC) $^@
 $(RIP) $< LDR_BASE fsd.inc >$^@
 $(DC) $<

freeldr.$(OUT): $(LDR_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

fat.$(OUT): $(FAT_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

fat.$(SOUT): $(FAT_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

ext2fs.$(OUT): $(EXT2FS_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

ext2fs.$(SOUT): $(EXT2FS_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

jfs.$(OUT): $(JFS_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

jfs.$(SOUT): $(JFS_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

iso9660.$(OUT): $(ISO9660_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

iso9660.$(SOUT): $(ISO9660_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

reiserfs.$(OUT): $(REISERFS_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

reiserfs.$(SOUT): $(REISERFS_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

minix.$(OUT): $(MINIX_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

minix.$(SOUT): $(MINIX_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

ufs2.$(OUT): $(UFS2_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

ufs2.$(SOUT): $(UFS2_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

vstafs.$(OUT): $(VSTAFS_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

vstafs.$(SOUT): $(VSTAFS_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

ffs.$(OUT): $(FFS_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

ffs.$(SOUT): $(FFS_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

xfs.$(OUT): $(XFS_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

xfs.$(SOUT): $(XFS_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

stage0.$(OUT): $(STAGE0_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

stage0.$(LOUT): $(STAGE0_LT_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="l" E=$(LOUT) OBJS="$<" link

stage0.$(SOUT): $(STAGE0_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

link: .SYMBOLIC .PROCEDURE
 @%create $(T)$(S).lnk
 @%append $(T)$(S).lnk system os2v2
 @%append $(T)$(S).lnk output raw offset=0x10000
 @%append $(T)$(S).lnk OPTION QUIET
 @%append $(T)$(S).lnk OPTION MAP=$(T)$(S).wmp
 @%append $(T)$(S).lnk OPTION NODEFAULTLIBS
 @%append $(T)$(S).lnk NAME $(T).$(E)
 @%append $(T)$(S).lnk ALIAS init=_init
!ifneq T stage0
!ifneq T freeldr
 @%append $(T)$(S).lnk ALIAS fs_mount_=$(T)_mount_
 @%append $(T)$(S).lnk ALIAS fs_dir_=$(T)_dir_
 @%append $(T)$(S).lnk ALIAS fs_read_=$(T)_read_
!else
 @%append $(T)$(S).lnk ALIAS _init=init_
 @%append $(T)$(S).lnk ALIAS cmain=cmain_
!endif
!else
 @%append $(T)$(S).lnk ALIAS mem_lower=_mem_lower
 @%append $(T)$(S).lnk ALIAS _biosdisk_int13_extensions=biosdisk_int13_extensions
 @%append $(T)$(S).lnk ALIAS _biosdisk_standard=biosdisk_standard
 @%append $(T)$(S).lnk ALIAS _check_int13_extensions=check_int13_extensions
 @%append $(T)$(S).lnk ALIAS _get_diskinfo_standard=get_diskinfo_standard
!endif
 @%append $(T)$(S).lnk PATH $(PATH)
 @for %%i in ($(OBJS)) do @%append $(T)$(S).lnk FILE %%i
 $(SAY) Linking $(T)$(S) $(LOG)
 $(LINKER) @$(T)$(S).lnk $(LOG)

fsys_fat.$(O): fsys_fat.c
 $(CC) -dFSYS_FAT -dSHIFT=0 $(COPT) -fo=$^@ $<

fsys_fat.$(SO): fsys_fat.c
 $(CC) -dFSYS_FAT -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

fsys_ext2fs.$(O): fsys_ext2fs.c
 $(CC) -dFSYS_EXT2FS -dSHIFT=0 $(COPT) -fo=$^@ $<

fsys_ext2fs.$(SO): fsys_ext2fs.c
 $(CC) -dFSYS_EXT2FS -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

fsys_jfs.$(O): fsys_jfs.c
 $(CC) -dFSYS_JFS -dSHIFT=0 $(COPT) -fo=$^@ $<

fsys_jfs.$(SO): fsys_jfs.c
 $(CC) -dFSYS_JFS -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

fsys_iso9660.$(O): fsys_iso9660.c
 $(CC) -dFSYS_ISO9660 -dSHIFT=0 $(COPT) -fo=$^@ $<

fsys_iso9660.$(SO): fsys_iso9660.c
 $(CC) -dFSYS_ISO9660 -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

fsys_reiserfs.$(O): fsys_reiserfs.c
 $(CC) -dFSYS_REISERFS -dSHIFT=0 $(COPT) -fo=$^@ $<

fsys_reiserfs.$(SO): fsys_reiserfs.c
 $(CC) -dFSYS_REISERFS -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

fsys_minix.$(O): fsys_minix.c
 $(CC) -dFSYS_MINIX -dSHIFT=0 $(COPT) -fo=$^@ $<

fsys_minix.$(SO): fsys_minix.c
 $(CC) -dFSYS_MINIX -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

fsys_ufs2.$(O): fsys_ufs2.c
 $(CC) -dFSYS_UFS2 -dSHIFT=0 $(COPT) -fo=$^@ $<

fsys_ufs2.$(SO): fsys_ufs2.c
 $(CC) -dFSYS_UFS2 -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

fsys_vstafs.$(O): fsys_vstafs.c
 $(CC) -dFSYS_VSTAFS -dSHIFT=0 $(COPT) -fo=$^@ $<

fsys_vstafs.$(SO): fsys_vstafs.c
 $(CC) -dFSYS_VSTAFS -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

fsys_ffs.$(O): fsys_ffs.c
 $(CC) -dFSYS_FFS -dSHIFT=0 $(COPT) -fo=$^@ $<

fsys_ffs.$(SO): fsys_ffs.c
 $(CC) -dFSYS_FFS -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

fsys_xfs.$(O): fsys_xfs.c
 $(CC) -dFSYS_XFS -dSHIFT=0 $(COPT) -fo=$^@ $<

fsys_xfs.$(SO): fsys_xfs.c
 $(CC) -dFSYS_XFS -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

func.$(O): func.c
 $(CC) -dSTAGE0 -dSHIFT=0 $(COPT) -fo=$^@ $<

func.$(LO): func.c
 $(CC) -dSTAGE0 -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(COPT) -fo=$^@ $<

func.$(SO): func.c
 $(CC) -dSTAGE0 -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

.c.$(O):
 $(CC) -dSHIFT=0 $(COPT) -fo=$^@ $<

.c.$(LO):
 $(CC) -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(COPT) -fo=$^@ $<

.c.$(SO):
 $(CC) -dSHIFT=$(SHIFT) $(COPT) -fo=$^@ $<

.asm.$(O):
 $(ASM) -dSHIFT=0 $(ASMOPT) -fo=$^@ $<

.asm.$(LO):
 $(ASM) -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(ASMOPT) -fo=$^@ $<

.asm.$(SO):
 $(ASM) -dSHIFT=$(SHIFT) $(ASMOPT) -fo=$^@ $<

.inc.h:
 $(AWK) -f inc2h.awk <$< >$^@

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(DC) $(TARGETS)
