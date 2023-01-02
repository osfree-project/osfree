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
ADD_COPT     = -s $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)lvmtk -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=$(MYDIR)include -i=$(MYDIR)..$(SEP)include -i=$(MYDIR) -i=$(MYDIR).. $(ADD_COPT)
ADD_ASMOPT   = $(DEFINES) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD -i=$(MYDIR)include  -i=$(MYDIR)..$(SEP)include -i=$(MYDIR) -i=$(MYDIR).. $(ADD_ASMOPT)

!ifndef DEST
DEST         = boot
!endif

!include $(%ROOT)/tools/mk/bootseq.mk

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
 @$(SAY) GENREL   $^. $(LOG)
 $(verbose)$(GENREL) $[*.mdl $[*.mds $(SHIFT) >$^@ $(LOG2)
!ifeq UNIX TRUE
 #@$(DC) $[*.mds
!else
 #@if exist $[*.mds @$(DC) $[*.mds $(BLACKHOLE)
!endif

.$(OUT).mdl:
 @$(SAY) RIPZ     $^. $(LOG)
!ifeq UNIX TRUE
 @$(DC) $^@
!else
 @if exist $^@ @$(DC) $^@ $(BLACKHOLE)
!endif
 $(verbose)$(RIP) $[@ $(MOD_BASE) $(%ROOT)$(SEP)bootseq$(SEP)loader$(SEP)include$(SEP)fsd.inc >$^@ $(LOG2)
!ifeq UNIX TRUE
 #@$(DC) $[@
!else
 #@if exist $[@ @$(DC) $[@ $(BLACKHOLE)
!endif

.$(SOUT).mds:
 @$(SAY) RIPZ     $^. $(LOG)
!ifeq UNIX TRUE
 @$(DC) $^@
!else
 @if exist $^@ @$(DC) $^@ $(BLACKHOLE)
!endif
 $(verbose)$(RIP) $[@ $(MOD_BASE) $(%ROOT)$(SEP)bootseq$(SEP)loader$(SEP)include$(SEP)fsd.inc $(SHIFT) >$^@ $(LOG2)
!ifeq UNIX TRUE
 #@$(DC) $[@
!else
 #@if exist $[@ @$(DC) $[@ $(BLACKHOLE)
!endif

link: $(PATH)$(T)$(S).lnk .SYMBOLIC .PROCEDURE .ALWAYS
 @$(SAY) LINK     $(T)$(S).$(E) $(LOG)
 $(verbose)$(LINKER) @$< $(LOG2)

$(PATH)$(T)$(S).lnk: $(OBJS) $(MYDIR)makefile .explicit
 @%create $^@
 @%append $^@ system os2v2 dll
 @%append $^@ output raw offset=0x10000
 @%append $^@ OPTION QUIET
 @%append $^@ OPTION START=start
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
 @%append $^@ LIBPATH $(ROOT)$(SEP)bin
 @%append $^@ LIBPATH $(BLD)lib
 @%append $^@ LIBRARY ldr_shared.lib
 @for %%i in ($(OBJS))   do @%append $^@ FILE %%i

!ifdef OBJS
#$(OBJS):: $(MYDIR)makefile
!endif

.c:   $(MYDIR)

.asm: $(MYDIR)

.c.$(O):
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=0 $(COPT) -fr=$^*.err -fo=$^@ $[@ $(LOG2)

.c.$(SO):
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=$(SHIFT) $(COPT) -fr=$^*.err -fo=$^@ $[@ $(LOG2)

.c.$(LO):
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC) -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(COPT) -fr=$^*.err -fo=$^@ $[@ $(LOG2)

.asm.$(O):
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dSHIFT=0 $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG2)

.asm.$(LO):
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dSHIFT=0 -dSTAGE1_5 -dNO_BLOCK_FILES $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG2)

.asm.$(SO):
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) -dSHIFT=$(SHIFT) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG2)

.inc.h:
 @$(SAY) INC2H    $^. $(LOG)
 $(verbose)$(AWK) -f $(ROOT)$(SEP)bin$(SEP)inc2h.awk <$[@ >$^@ $(LOG2)

#
# See $(%ROOT)/mk/genrules.mk for details
#
gen_compile_rules_wrapper: $(MYDIR)$(file) .symbolic
!ifeq sh
 # compile rules for ordinary files
!ifdef __UNIX__
 @$(MAKE) $(MAKEOPT) file=$[. ext=$(file:$[&=) e='.$$$$$$$$(O)'  sh=$(sh) basename=$[& gen_compile_rules
!else
 @$(MAKE) $(MAKEOPT) file=$[. ext=$(file:$[&=) e=.$$$$$$$$(O)  sh=$(sh) basename=$[& gen_compile_rules
!endif
!else
 # compile rules for shifted files
!ifdef __UNIX__
 @$(MAKE) $(MAKEOPT) file=$[. ext=$(file:$[&=) e='.$$$$$$$$(SO)' sh=$(sh) basename=$[& gen_compile_rules
!else
 @$(MAKE) $(MAKEOPT) file=$[. ext=$(file:$[&=) e=.$$$$$$$$(SO) sh=$(sh) basename=$[& gen_compile_rules
!endif
!endif

gen_deps_wrapper: .symbolic
 # file.rel: file.mdl file.mds
!ifdef __UNIX__
 @for %i in ($(bbx)) do @$(MAKE) $(MAKEOPT) file=%i trgt='$$$$(PATH)$$(file).rel' &
   deps='$+$$$$$$$$(PATH)$$$$(file).mdl $$$$$$$$(PATH)$$$$(file).mds$-' gen_deps
 # file.fsd: file.$(OUT)
 @for %i in ($(bbx)) do @$(MAKE) $(MAKEOPT) file=%i trgt='$$$$(PATH)$$(file).mdl' &
   deps='$+$$$$$$$$(PATH)$$$$(file).$$$$$$$$(OUT)$-' gen_deps
 # file.fss: file.$(SOUT)
 @for %i in ($(bbx)) do @$(MAKE) $(MAKEOPT) file=%i trgt='$$$$(PATH)$$(file).mds' &
   deps='$+$$$$$$$$(PATH)$$$$(file).$$$$$$$$(SOUT)$-' gen_deps
!else
 @for %i in ($(bbx)) do @$(MAKE) $(MAKEOPT) file=%i trgt=$$$$(PATH)$$(file).rel &
   deps="$+$$$$$$$$(PATH)$$$$(file).mdl $$$$$$$$(PATH)$$$$(file).mds$-" gen_deps
 # file.fsd: file.$(OUT)
 @for %i in ($(bbx)) do @$(MAKE) $(MAKEOPT) file=%i trgt=$$$$(PATH)$$(file).mdl &
   deps=$+$$$$$$$$(PATH)$$$$(file).$$$$$$$$(OUT)$- gen_deps
 # file.fss: file.$(SOUT)
 @for %i in ($(bbx)) do @$(MAKE) $(MAKEOPT) file=%i trgt=$$$$(PATH)$$(file).mds &
   deps=$+$$$$$$$$(PATH)$$$$(file).$$$$$$$$(SOUT)$- gen_deps
!endif

!endif
