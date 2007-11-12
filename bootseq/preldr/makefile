#
# Makefile for microFSD's and stage0.
# 07/10/04 valerius
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS      = 1       # Use 32-bit C compiler
DEFINES      = -dNO_DECOMPRESSION -dSTAGE1_5 -dNO_BLOCK_FILES # -dOS2 -d__WATCOM__
ADD_COPT     = -q $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=include -i=. -i=..
ADD_ASMOPT   = -q $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=include -i=. -i=..

DIR          = preldr
MYDIR        = $(ROOT)$(SEP)bootseq$(SEP)$(DIR)$(SEP)
PATH         = $(MYDIR)

!include $(%ROOT)/mk/bootseq.mk

RIP           = ripzeroes.cmd

FS_OBJS       = start.$(O) fsys.$(O) dummy.$(O)

FAT_OBJS      = $(FS_OBJS) fsys_fat.$(O)
EXT2FS_OBJS   = $(FS_OBJS) fsys_ext2fs.$(O)
JFS_OBJS      = $(FS_OBJS) fsys_jfs.$(O)
ISO9660_OBJS  = $(FS_OBJS) fsys_iso9660.$(O)
REISERFS_OBJS = $(FS_OBJS) fsys_reiserfs.$(O)

STAGE0_OBJS   = segord.$(O) init.$(O) modesw.$(O) biosdisk.$(O) filesys.$(O) &
                video.$(O) stage0_fs.$(O) func.$(O) bios.$(O) end.$(O)

OUT           = bin
TARGETS       = fsd.h fat.fsd ext2fs.fsd jfs.fsd iso9660.fsd preldr0 # reiserfs.fsd

all: $(TARGETS) .SYMBOLIC

.$(OUT).fsd:
 $(DC) $^@
 $(RIP) UFSD_BASE   fsd.inc $< $^@
 $(DC) $<

preldr0: stage0.$(OUT)
 $(DC) $^@
 $(RIP) STAGE0_BASE fsd.inc $< $^@
 $(DC) $<

fat.$(OUT): $(FAT_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& OBJS="$<" link

ext2fs.$(OUT): $(EXT2FS_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& OBJS="$<" link

jfs.$(OUT): $(JFS_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& OBJS="$<" link

iso9660.$(OUT): $(ISO9660_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& OBJS="$<" link

reiserfs.$(OUT): $(REISERFS_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& OBJS="$<" link

stage0.$(OUT): $(STAGE0_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& OBJS="$<" link

link: .SYMBOLIC .PROCEDURE
 @%create $(T).lnk
 @%append $(T).lnk system os2v2
 @%append $(T).lnk output raw offset=0x10000
 @%append $(T).lnk OPTION QUIET
 @%append $(T).lnk OPTION MAP=$(T).wmp
 @%append $(T).lnk OPTION NODEFAULTLIBS
 @%append $(T).lnk NAME $(T)
 @%append $(T).lnk ALIAS init=_init
!ifneq T stage0
 @%append $(T).lnk ALIAS fs_mount_=$(T)_mount_
 @%append $(T).lnk ALIAS fs_dir_=$(T)_dir_
 @%append $(T).lnk ALIAS fs_read_=$(T)_read_
!else
 @%append $(T).lnk ALIAS _biosdisk_int13_extensions=biosdisk_int13_extensions
 @%append $(T).lnk ALIAS _biosdisk_standard=biosdisk_standard
 @%append $(T).lnk ALIAS _check_int13_extensions=check_int13_extensions
 @%append $(T).lnk ALIAS _get_diskinfo_standard=get_diskinfo_standard
!endif
 @%append $(T).lnk PATH $(PATH)
 @for %%i in ($(OBJS)) do @%append $(T).lnk FILE %%i
 $(SAY) Linking $(T) $(LOG)
 $(LINKER) @$(T).lnk $(LOG)

fsys_fat.$(O): fsys_fat.c
 $(CC) -dFSYS_FAT $(COPT) -fo=$^@ $<

fsys_ext2fs.$(O): fsys_ext2fs.c
 $(CC) -dFSYS_EXT2FS $(COPT) -fo=$^@ $<

fsys_jfs.$(O): fsys_jfs.c
 $(CC) -dFSYS_JFS $(COPT) -fo=$^@ $<

fsys_iso9660.$(O): fsys_iso9660.c
 $(CC) -dFSYS_ISO9660 $(COPT) -fo=$^@ $<

fsys_reiserfs.$(O): fsys_reiserfs.c
 $(CC) -dFSYS_REISERFS $(COPT) -fo=$^@ $<

func.$(O): func.c
 $(CC) -dSTAGE0 $(COPT) -fo=$^@ $<

.inc.h:
 $(AWK) -f inc2h.awk <$< >$^@

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(DC) $(TARGETS)
