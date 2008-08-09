#
# Makefile for microFSD's and stage0.
# 07/10/04 valerius
#

#TARGETS          = minilibc.lib preldr0 preldr0_lite freeldr preldr0s &
#                   bt_linux bt_chain &
#                   console.trm hercules.trm serial.trm &
#                   console.trs hercules.trs serial.trs &
# preldr0.rel &
#                   console.rel hercules.rel serial.rel

#
# This is for this file to be not included twice
#
!ifneq _loader_mk_ 1
_loader_mk_ = 1

32_BITS      = 1       # Use 32-bit C compiler
DEFINES      = -dNO_DECOMPRESSION # -dSTAGE1_5 -dNO_BLOCK_FILES -dOS2 -d__WATCOM__
ADD_COPT     = -s $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=$(MYDIR)include -i=$(MYDIR)..$(SEP)include -i=. -i=..
ADD_ASMOPT   = $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=$(MYDIR)include  -i=$(MYDIR)..$(SEP)include -i=. -i=..

all: install endrule

install: build

build: prereq .SYMBOLIC
 @$(MAKE) $(MAKEOPT) -f $(mf) $(TARGETS)

!include $(%ROOT)/mk/bootseq.mk
!include $(%ROOT)/mk/genrules.mk

CLEANMASK    = $(CLEANMASK) *.sob *.lob

SHIFT = 0x100

SO           = sob
LO           = lob

OUT          = bin
SOUT         = sbi
LOUT         = lbi

.SUFFIXES:
.SUFFIXES: .rel .bbx .bbs .$(OUT) .$(LOUT) .$(SOUT) .lib .$(LO) .$(SO) .$(O) .c .asm .h .inc

BT_LINUX_OBJS    = linux.$(O) linuxc.$(O) modesw-npl.$(O) cmdline.$(O) &
                   wrap.$(O) end.$(O)
BT_CHAIN_OBJS    = chain.$(O) chainc.$(O) setdev.$(O) modesw-npc.$(O)  &
                   cmdline.$(O) wrap.$(O) end.$(O)

STAGE0_OBJS      = segord.$(O) init.$(O) modesw.$(O) asmcode.$(O) apm.$(O) biosdisk.$(O) filesys.$(O) &
                   video.$(O) vbe.$(O) common.$(O) stage0_fs.$(O) func.$(O) setdev.$(O) cmdline.$(O)  &
                   dskaccess.$(O) part.$(O) cfgparse.$(O) idt.$(O) bios.$(O) end.$(O)

STAGE0_SH_OBJS   = segord.$(SO) init.$(SO) modesw.$(SO) asmcode.$(SO) apm.$(SO) biosdisk.$(SO) filesys.$(SO) &
                   video.$(SO) vbe.$(SO) common.$(SO) stage0_fs.$(SO) func.$(SO) setdev.$(SO) cmdline.$(SO) &
                   dskaccess.$(SO) part.$(SO) cfgparse.$(SO) idt.$(SO) bios.$(SO) end.$(SO)

STAGE0_LT_OBJS   = segord.$(LO) init.$(LO) modesw.$(LO) asmcode.$(LO) apm.$(LO) biosdisk.$(LO) filesys.$(LO) &
                   dskaccess.$(LO) part.$(LO) video.$(LO) common.$(LO) stage0_fs.$(LO) func.$(LO) setdev.$(O) bios.$(LO) end.$(LO)

LDR_OBJS         = ldrstart.$(O) loader.$(O) wrap.$(O) commands.$(O) &
                   boot.$(O) cmdline.$(O) cfgparse-l.$(O) #varsubst.$(O)

!ifneq PATH ""
PATH = $(BLD)$(PATH)
!endif

preldr0.rel: preldr0 preldr0s

.bbx: $(PATH)

.bbs: $(PATH)

.$(OUT):  $(PATH)

.$(SOUT): $(PATH)

preldr0.rel: preldr0
 $(GENREL) $^& $^&s $(SHIFT) >$^@
 $(DC) $[*s

.bbx.rel
 $(GENREL) $[*.bbx $[*.bbs $(SHIFT) >$^@
 $(DC) $[*.bbs

bt_linux: bt_linux.$(OUT)
 $(DC) $^@
 $(RIP) $< KERN_BASE $(MYDIR)..$(SEP)include$(SEP)mb_etc.inc >$^@
 $(DC) $<

bt_chain: bt_chain.$(OUT)
 $(DC) $^@
 $(RIP) $< KERN_BASE $(MYDIR)..$(SEP)include$(SEP)mb_etc.inc >$^@
 $(DC) $<

.$(OUT).bbx:
 $(DC) $^@
 $(RIP) $[@ EXT_BUF_BASE $(MYDIR)..$(SEP)include$(SEP)fsd.inc >$^@
 $(DC) $[@

.$(SOUT).bbs:
 $(DC) $^@
 $(RIP) $[@ EXT_BUF_BASE $(MYDIR)..$(SEP)include$(SEP)fsd.inc $(SHIFT) >$^@
 $(DC) $[@

preldr0: stage0.$(OUT)
 $(DC) $^@
 $(RIP) $< STAGE0_BASE $(MYDIR)..$(SEP)include$(SEP)fsd.inc >$^@
 $(DC) $<

preldr0_lite: stage0.$(LOUT)
 $(DC) $^@
 $(RIP) $< STAGE0_BASE $(MYDIR)..$(SEP)include$(SEP)fsd.inc >$^@
 $(DC) $<

preldr0s: stage0.$(SOUT)
 $(DC) $^@
 $(RIP) $< STAGE0_BASE $(MYDIR)..$(SEP)include$(SEP)fsd.inc $(SHIFT) >$^@
 $(DC) $<

freeldr: freeldr.$(OUT)
 $(DC) $^@
 $(RIP) $< LDR_BASE $(MYDIR)..$(SEP)include$(SEP)fsd.inc >$^@
 $(DC) $<

bt_linux.$(OUT): $(BT_LINUX_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

bt_chain.$(OUT): $(BT_CHAIN_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

freeldr.$(OUT): $(LDR_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

stage0.$(OUT): $(STAGE0_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="" E=$(OUT) OBJS="$<" link

stage0.$(LOUT): $(STAGE0_LT_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="l" E=$(LOUT) OBJS="$<" link

stage0.$(SOUT): $(STAGE0_SH_OBJS)
 $(MAKE) $(MAKEOPT) T=$^& S="s" E=$(SOUT) OBJS="$<" link

link: $(PATH)$(T)$(S).lnk .SYMBOLIC .PROCEDURE
 $(SAY) Linking $< $(LOG)
 $(LINKER) @$< $(LOG)

$(PATH)$(T)$(S).lnk: .SYMBOLIC
 @%create $^@
 @%append $^@ system os2v2
 @%append $^@ output raw offset=0x10000
 @%append $^@ OPTION QUIET
 @%append $^@ OPTION MAP=$^*.wmp
 @%append $^@ OPTION NODEFAULTLIBS
 @%append $^@ NAME $(PATH)$(T).$(E)
 @%append $^@ ALIAS init=_init
!ifneq T stage0
!ifeq FS 1
 @%append $^@ ALIAS fs_mount_=$(T)_mount_
 @%append $^@ ALIAS fs_dir_=$(T)_dir_
 @%append $^@ ALIAS fs_read_=$(T)_read_
!else
 @%append $^@ ALIAS _init=init_
 @%append $^@ ALIAS cmain=cmain_
!endif
!else
 @%append $^@ ALIAS mem_lower=_mem_lower
 @%append $^@ ALIAS _biosdisk_int13_extensions=biosdisk_int13_extensions
 @%append $^@ ALIAS _biosdisk_standard=biosdisk_standard
 @%append $^@ ALIAS _check_int13_extensions=check_int13_extensions
 @%append $^@ ALIAS _get_diskinfo_standard=get_diskinfo_standard
!endif
 @%append $^@ LIBPATH $(PATH)..$(SEP)minilibc
 @%append $^@ LIBRARY minilibc.lib
 @for %%i in ($(OBJS)) do @%append $^@ FILE %%i

.c:   $(MYDIR)

.asm: $(MYDIR)

func.$(O): func.c
 $(CC) -dSTAGE0 -dSHIFT=0 $(COPT) -fr=$^& -fo=$^@ $<

func.$(LO): func.c
 $(CC) -dSTAGE0 -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(COPT) -fr=$^& -fo=$^@ $<

func.$(SO): func.c
 $(CC) -dSTAGE0 -dSHIFT=$(SHIFT) $(COPT) -fr=$^& -fo=$^@ $<

modesw-npl.$(O): modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x8000 -dSHIFT=0 $(ASMOPT) -fr=$^& -fo=$^@ $<

linux.$(O): linux.asm
 $(ASM) -dSHIFT=0 -dREAL_BASE=0x8000 $(ASMOPT) -fr=$^& -fo=$^@ $<

modesw-npc.$(O): modesw.asm
 $(ASM) -dNO_PROT -dREAL_BASE=0x90000 -dSHIFT=0 $(ASMOPT) -fr=$^& -fo=$^@ $<

chain.$(O): chain.asm
 $(ASM) -dSHIFT=0 -dREAL_BASE=0x90000 $(ASMOPT) -fr=$^& -fo=$^@ $<

cfgparse-l.$(O): cfgparse.c
 $(CC) -dLOADER $(COPT) -fr=$^& -fo=$^@ $<

cfgparse.$(O): cfgparse.c

.c.$(O):
 $(CC) -dSHIFT=0 $(COPT) -fr=$^&.err -fo=$^@ $<

.c.$(LO):
 $(CC) -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(COPT) -fr=$^&.err -fo=$^@ $<

.c.$(SO):
 $(CC) -dSHIFT=$(SHIFT) $(COPT) -fr=$^&.err -fo=$^@ $<

.asm.$(O):
 $(ASM) -dSHIFT=0 $(ASMOPT) -fr=$^&.err -fo=$^@ $<

.asm.$(LO):
 $(ASM) -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(ASMOPT) -fr=$^&.err -fo=$^@ $<

.asm.$(SO):
 $(ASM) -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^&.err -fo=$^@ $<

.inc.h:
 $(AWK) -f inc2h.awk <$< >$^@

#
# See $(%ROOT)/mk/genrules.mk for details
#
gen_compile_rules_wrapper: $(MYDIR)$(file) .SYMBOLIC
!ifeq sh
 # compile rules for ordinary files
 @$(MAKE) $(MAKEOPT) file=$[. ext=$(file:$[&=) e=.$$$$$$$$(O)  basename=$[& gen_compile_rules
!else
 # compile rules for shifted files
 @$(MAKE) $(MAKEOPT) file=$[. ext=$(file:$[&=) e=.$$$$$$$$(SO) basename=$[& gen_compile_rules
!endif

gen_deps_wrapper:
 # file.rel: file.bbx file.bbs
 @for %i in ($(bbx)) do @$(MAKE) $(MAKEOPT) file=%i trgt=$$$$(PATH)$$(file).rel &
   deps="$+$$$$$$$$(PATH)$$$$(file).bbx $$$$$$$$(PATH)$$$$(file).bbs$-" gen_deps
 # file.fsd: file.$(OUT)
 @for %i in ($(bbx)) do @$(MAKE) $(MAKEOPT) file=%i trgt=$$$$(PATH)$$(file).bbx &
   deps="$+$$$$$$$$(PATH)$$$$(file).$$$$$$$$(OUT)$-" gen_deps
 # file.fss: file.$(SOUT)
 @for %i in ($(bbx)) do @$(MAKE) $(MAKEOPT) file=%i trgt=$$$$(PATH)$$(file).bbs &
   deps="$+$$$$$$$$(PATH)$$$$(file).$$$$$$$$(SOUT)$-" gen_deps

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 @$(MDHIER) $(FILESDIR)$(SEP)boot
 @for %i in ($(PROJ)) do @if exist %i $(CP) %i $(FILESDIR)$(SEP)boot

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(DC) $(TARGETS)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

!endif
