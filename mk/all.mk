#
# OS/3 (osFree) project
# common make macros.
# (c) osFree project
# valerius, 2006/10/30
#

!ifndef __all_mk__
!define __all_mk__

all: install

install: build

!include $(%ROOT)/mk/dirs.mk
!include $(%ROOT)/mk/genrules.mk

# build and install each target in sequence
build: precopy prereq # .SYMBOLIC
 @$(MAKE) $(MAKEOPT) -f $(mf) $(TARGETS)

TRG  =

#
# Preprocessor defines
#
C_DEFS    = -zq -d__WATCOM__
ASM_DEFS  = -zq -d__WATCOM__

#
# ADD_COPT and ADD_ASMOPT are defined in
# a file which includes this file.
#
#!ifndef __bootseq_mk__ # if this file is not included from bootseq.mk
#!ifeq 32_BITS 1
COPT      = $(C_DEFS) $(ADD_COPT) &
            -i=$(MYDIR) -i=$(MYDIR).. &
            -i=$(PATH)  -i=$(PATH)..
ASMOPT    = $(ASM_DEFS) $(ADD_ASMOPT)
#!else
#COPT      = $(C_DEFS) -i=$(ROOT)$(SEP)include$(SEP)os3 -i=. -i=.. -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)pm -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)GDlib -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)zlib -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)gbm $(ADD_COPT)
#ASMOPT    = $(ASM_DEFS) $(ADD_ASMOPT)
#!endif
#!else
#COPT      = $(C_DEFS) -i=$(ROOT)$(SEP)include -i=$(ROOT)$(SEP)include$(SEP)uFSD  -i=. -i=.. $(ADD_COPT)
#ASMOPT    = $(ASM_DEFS) -i=. -i=.. $(ADD_ASMOPT)
#!endif

#
# Tools:
#
!ifeq 32_BITS 1
CC        = @wcc386
CPPC      = @wpp386
!else
CC        = @wcc
CPPC      = @wpp
!endif

ASM       = @wasm

LINKER    = @wlink
LINKOPT   = op q $(ADD_LINKOPT)

LIB       = @wlib
LIBOPT    = -q -n -fo

# Don't add @ sign here. Will break build system
MAKE      = wmake
MAKEOPT   = -h

PC        = ppc386
PCOPT     = -Sg2h

DD        = dd

SED       = sed
AWK       = @awk
DOX       = doxygen

# Watcom 1.7 RC has bug with resource storing. Resursed not just added
# but replaced. So, we still use OS/2 TK RC.EXE
#RC        = @wrc -q
#RCOPT     = -bt=OS2

RC        = wrc
RCOPT     = $(RCOPT) $(ADD_RCOPT)

MC        = mkmsgf

HC        = ipfc

CD        = $(REXX) cdir.cmd
MDHIER    = $(REXX) mdhier.cmd

GENE2FS   = genext2fs

SYS       = sys

# A command to add a list of object
# files to a linker script
ADDFILES_CMD = @for %i in ($(OBJS)) do @%append $^@ FILE %i

#
# Extensions to clean up
#
CLEANMASK = *.dlo *.lnk *.map *.obj *.err *.log *.bak *.lib *.com *.sym *.bin *.exe *.dll *.wmp *.ppu *.rst *.res $(CLEAN_ADD)

!ifeq UNIX FALSE                 # Non-unix

!ifeq ENV OS/2
COMSPEC   = $(OS_SHELL)          # Shell
OS2_SHELL = $(OS_SHELL)          #
RN  = @move                      # Rename command
!else ifeq ENV Windows
RN  = @ren                       # Rename command
!else ifeq ENV WIN32
RN  = @ren                       # Rename command
!endif

SEP       = \                  # dir components separator
PS        = ;                  # paths separator
O         = obj                  # Object Extension differs from Linux to OS/2
DC        = @del                 # Delete command is rm on linux and del on OS/2
CP        = @copy                # Copy command
SAY       = @echo                # Echo message
MKBIN     = $(REXX) mkbin.cmd
GENHDD    = $(REXX) genhdd.cmd
GENFDD    = $(REXX) genfdd.cmd
FINDFILE  = $(REXX) findfile.cmd
if_not_exist_mkdir = if_not_exist_mkdir.cmd

CLEAN_CMD    = @for %i in ($(CLEANMASK)) do @if exist $(PATH)%i $(DC) $(PATH)%i $(BLACKHOLE)

!ifeq ENV Windows
NULL      = nul
!else ifeq ENV WIN32
NULL      = nul
!else
NULL      = \dev\nul
!endif
BLACKHOLE = 2>&1 >$(NULL)

!else ifeq UNIX TRUE             # UNIX

SEP       = /                  # dir components separator
PS        = :                  # paths separator
O         = obj                  # Object Extension differs from Linux to OS/2
DC        = rm -f                # Delete command is rm on linux and del on OS/2
CP        = cp                   # Copy command
RN        = mv                   # Rename command
SAY       = echo                 # Echo message
MKBIN     = mkbin
GENHDD    = genhdd
GENFDD    = genfdd
FINDFILE  = findfile

if_not_exist_mkdir = ./if_not_exist_mkdir.sh

CLEAN_CMD    = @for %i in ($(CLEANMASK)) do @if exist $(PATH)%i $(DC) $(PATH)%i $(BLACKHOLE)

NULL      = /dev/null
BLACKHOLE = 2>&1 >$(NULL)

!endif

MKDIR     = @mkdir
MAPSYM    = @mapsym

LOG       = # >$(ROOT)$(SEP)compile.log 2>&1

!ifndef OBJS
!ifdef  srcfiles
p = $(PATH)
e = .$(O)
OBJS = $+$(srcfiles)$-
!endif
!endif

SUF = $(SUF) .sym .exe .dll .lib .res .lnk .inf .obj .c .cpp .asm .h .y .l .hpp .inc .rc .pas .pp .ipf .map .wmp .rexx .cmd

.SUFFIXES:
.SUFFIXES: $(SUF)

.l:   $(MYDIR)

.y:   $(MYDIR)

.wmp: $(PATH)

.map: $(PATH)

.rc:  $(MYDIR)

.res: $(PATH)

.lnk: $(PATH)

.rexx: $(MYDIR)

.cmd: $(MYDIR)

.obj: $(PATH)

.c:   $(PATH)

.h:   $(PATH)

.ipf: $(MYDIR)

.l.c: .AUTODEPEND
 $(DC) $^@
 lex -t $[@ >$^@

.y.c: .AUTODEPEND
 $(DC) $^*.h
 $(DC) $^*.c
 yacc -y -d -o $^@ $[@

.c:   $(MYDIR)

.cpp: $(MYDIR)

.h:   $(MYDIR)

.asm: $(MYDIR)

.pas: $(MYDIR)

.pp:  $(MYDIR)

.c.obj: .AUTODEPEND
 $(SAY) Compiling $[. $(LOG)
 $(CC)  $(COPT)   -fr=$^*.err -fo=$^@ $[@ $(LOG)

.asm.obj: .AUTODEPEND
 $(SAY) Assembling $[. $(LOG)
 $(ASM) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

.cpp.obj: .AUTODEPEND
 $(SAY) Compiling $[. $(LOG)
 $(CPPC) $(COPT)  -fr=$^*.err -fo=$^@ $[@ $(LOG)

.wmp.map: .AUTODEPEND
 $(SAY) Converting Watcom MAP to VAC MAP $[. $(LOG)
 $(AWK) -f $(FILESDIR)$(SEP)tools$(SEP)mapsym.awk <$< >$(PATH)$^@

.map.sym: .AUTODEPEND
 $(SAY) Converting VAC MAP to OS/2 SYM $[. $(LOG)
 $(MAPSYM) $[@
 $(RN) $^. $^:

.ipf.inf: .symbolic
 $(SAY) Compiling IPF source file $[.... $(LOG)
 $(HC) -i $[@ $^@

.rc.res: .AUTODEPEND
 $(SAY) Compiling resource file $[.... $(LOG)
 $(RC) $(RCOPT) $[@ -fo=$^@ -r

.pas.exe: .symbolic
 $(SAY) Compiling $[....
 $(PC) $(PCOPT) -o$^. -FE$^: -Fe$^: $[@

.pp.exe: .symbolic
 $(SAY) Compiling $[....
 $(PC) $(PCOPT) -o$^. -FE$^: -Fe$^: $[@

.lnk.exe: .symbolic
 $(SAY) Linking $^.... $(LOG)
 $(LINKER) $(LINKOPT) @$[@ $(LOG)

.rexx.exe: .AUTODEPEND
  $(SAY) Wrapping REXX code $[....
  rexxwrapper -program=$^* -rexxfiles=$^*.rexx -srcdir=$(%ROOT)$(SEP)tools$(SEP)rexxwrap -compiler=wcc -interpreter=os2rexx -intlib=rexx.lib -intincdir=$(%WATCOM)$(SEP)h$(SEP)os2 -compress

#
# "$(MAKE) subdirs" enters each dir in $(DIRS)
# and does $(MAKE) $(TARGET) in each dir:
#
subdirs: .SYMBOLIC
 @for %%i in ($(DIRS)) do @cd %%i && cd && $(MAKE) $(MAKEOPT) $(TARGET) && cd ..

dirhier: .SYMBOLIC
 @$(MDHIER) $(PATH)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
!ifeq INSTALL_PRE 1
 @$(MAKE) $(MAKEOPT) install_pre
!endif
!ifneq DEST
 @$(MDHIER) $(DEST)
 @for %i in ($(TARGETS)) do @if exist %i $(CP) %i $(DEST)
!endif
!ifeq INSTALL_ADD 1
 @$(MAKE) $(MAKEOPT) install_add
!endif

precopy: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) -f $(ROOT)$(SEP)tools$(SEP)scripts$(SEP)makefile copy

.error
 @$(SAY) Error (!)
 @%abort

!ifndef WRAPPERS
# define if gen_compile_rules_wrapper and gen_deps_wrapper
# are not defined
gen_compile_rules_wrapper: .SYMBOLIC
 @%null

gen_deps_wrapper: .SYMBOLIC
 @%null

!endif

!endif
