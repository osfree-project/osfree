#
# Makefile for microFSD's and stage0.
# 07/10/04 valerius
#

#
# This is for this file to be not included twice
#
!ifndef __loader_mk__
!define __loader_mk__

# define if gen_compile_rules_wrapper and gen_deps_wrapper
# are defined
!define WRAPPERS

CLEAN_ADD = *.mdl *.rel *.sob *.lob $(CLEAN_ADD)

32_BITS      = 1       # Use 32-bit C compiler
DEFINES      = -dNO_DECOMPRESSION # -dSTAGE1_5 -dNO_BLOCK_FILES -dOS2 -d__WATCOM__
ADD_COPT     = -s $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=$(MYDIR)include -i=$(MYDIR)..$(SEP)include -i=$(MYDIR) -i=$(MYDIR).. $(ADD_COPT)
ADD_ASMOPT   = $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=$(MYDIR)include  -i=$(MYDIR)..$(SEP)include -i=$(MYDIR) -i=$(MYDIR).. $(ADD_ASMOPT)

!ifndef DEST
DEST         = boot
!endif

!include $(%ROOT)/mk/bootseq.mk

SHIFT = 0x100

SO           = sob
LO           = lob

OUT          = bin
SOUT         = sbi
LOUT         = lbi

.SUFFIXES:
.SUFFIXES: .rel .mdl .mds .$(OUT) .$(LOUT) .$(SOUT) .lib .$(LO) .$(SO) .$(O) .c .asm .h .inc

.c:   $(MYDIR)..

.h:   $(MYDIR)..

.asm: $(MYDIR)..

#preldr0.rel: preldr0 preldr0s

.rel: $(PATH)

.mdl: $(PATH)

.mds: $(PATH)

.$(OUT):  $(PATH)

.$(SOUT): $(PATH)

.mdl.rel
 $(GENREL) $[*.mdl $[*.mds $(SHIFT) >$^@
 $(DC) $[*.mds

.$(OUT).mdl:
 $(DC) $^@
 $(RIP) $[@ $(MOD_BASE) $(%ROOT)bootseq$(SEP)loader$(SEP)include$(SEP)fsd.inc >$^@
 $(DC) $[@

.$(SOUT).mds:
 $(DC) $^@
 $(RIP) $[@ $(MOD_BASE) $(%ROOT)bootseq$(SEP)loader$(SEP)include$(SEP)fsd.inc $(SHIFT) >$^@
 $(DC) $[@

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
!ifneq T preldr0
!ifneq T preldr_mini
!ifeq FS 1
 @%append $^@ ALIAS fs_mount_=$(T)_mount_
 @%append $^@ ALIAS fs_dir_=$(T)_dir_
 @%append $^@ ALIAS fs_read_=$(T)_read_
!else
 @%append $^@ ALIAS _init=init_
 @%append $^@ ALIAS cmain=cmain_
!endif
!else
 #@%append $^@ ALIAS mem_lower=_mem_lower
 #@%append $^@ ALIAS _biosdisk_int13_extensions=biosdisk_int13_extensions
 #@%append $^@ ALIAS _biosdisk_standard=biosdisk_standard
 #@%append $^@ ALIAS _check_int13_extensions=check_int13_extensions
 #@%append $^@ ALIAS _get_diskinfo_standard=get_diskinfo_standard
!endif
!endif
!ifdef ALIASES
 @%append $^@ ALIAS  $(ALIASES)
!endif
!ifdef ORDER
 @%append $^@ $(ORDER)
!endif
 @%append $^@ LIBPATH $(BLD)lib
 @%append $^@ LIBRARY ldr_shared.lib
 @for %%i in ($(OBJS))   do @%append $^@ FILE %%i

.c:   $(MYDIR)

.asm: $(MYDIR)

.c.$(O):
 $(CC) -dSHIFT=0 $(COPT) -fr=$^*.err -fo=$^@ $[@

.c.$(SO):
 $(CC) -dSHIFT=$(SHIFT) $(COPT) -fr=$^*.err -fo=$^@ $[@

.c.$(LO):
 $(CC) -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(COPT) -fr=$^*.err -fo=$^@ $[@

.asm.$(O):
 $(ASM) -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

.asm.$(LO):
 $(ASM) -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

.asm.$(SO):
 $(ASM) -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@

.inc.h:
 $(AWK) -f $(ROOT)$(SEP)bin$(SEP)inc2h.awk <$[@ >$^@

#
# See $(%ROOT)/mk/genrules.mk for details
#
gen_compile_rules_wrapper: $(MYDIR)$(file) .SYMBOLIC
!ifeq sh
 # compile rules for ordinary files
 $(MAKE) $(MAKEOPT) file=$[. ext=$(file:$[&=) e='.$$$$$$$$(O)'  sh=$(sh) basename=$[& gen_compile_rules
!else
 # compile rules for shifted files
 $(MAKE) $(MAKEOPT) file=$[. ext=$(file:$[&=) e='.$$$$$$$$(SO)' sh=$(sh) basename=$[& gen_compile_rules
!endif

gen_deps_wrapper:
 # file.rel: file.mdl file.mds
 @for %i in ($(bbx)) do $(MAKE) $(MAKEOPT) file=%i trgt='$$$$(PATH)$$(file).rel' &
   deps='$+$$$$$$$$(PATH)$$$$(file).mdl $$$$$$$$(PATH)$$$$(file).mds$-' gen_deps
 # file.fsd: file.$(OUT)
 @for %i in ($(bbx)) do $(MAKE) $(MAKEOPT) file=%i trgt='$$$$(PATH)$$(file).mdl' &
   deps='$+$$$$$$$$(PATH)$$$$(file).$$$$$$$$(OUT)$-' gen_deps
 # file.fss: file.$(SOUT)
 @for %i in ($(bbx)) do $(MAKE) $(MAKEOPT) file=%i trgt='$$$$(PATH)$$(file).mds' &
   deps='$+$$$$$$$$(PATH)$$$$(file).$$$$$$$$(SOUT)$-' gen_deps

!endif
