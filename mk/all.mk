#
# OS/3 (osFree) project
# common make macros.
# (c) osFree project
# valerius, 2006/10/30
#

!ifndef __all_mk__
!define __all_mk__

all: install .symbolic

install: build

test: .symbolic
  echo $(MYDIR)
  echo $(PATH)
  echo $(mfh)

!include $(%ROOT)/mk/dirs.mk
!include $(%ROOT)/mk/genrules.mk


# build and install each target in sequence
build: precopy prereq .SYMBOLIC
 $(MAKE) $(MAKEOPT) -f $(mf) $(TARGETS)

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
COPT      = $(C_DEFS) $(ADD_COPT) &
            -i=$(MYDIR) -i=$(MYDIR).. &
            -i=$(PATH)  -i=$(PATH).. 
#            -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until UniAPI headers will be ready
ASMOPT    = $(ASM_DEFS) $(ADD_ASMOPT)
C16OPT    = -nt=_TEXT16 -nd=D $(ADD_COPT)

#
# Tools:
#
!ifeq 32_BITS 1
CC        = wcc386
CPPC      = @wpp386
!else
CC        = @wcc
CPPC      = @wpp
!endif

CC16      = @wcc
CPPC16    = @wpp

ASM       = @wasm

LINKER    = @wlink
# Note by valerius:
# don't add the following option to all.mk
# -----op internalrelocs----
# as it breaks all non-LX executables, for ex., bootsectors
LINKOPT   = op q libpath $(%ROOT)$(SEP)build$(SEP)lib $(ADD_LINKOPT)

LIB       = @wlib
LIBOPT    = -q -n -fo

# Don't add @ sign here. Will break build system
MAKE      = wmake
MAKEOPT   = -h

PC        = ppc386
PCOPT     = -Sg2h $(ADD_PCOPT)

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

MC        = mkmsgf.exe

HC        = wipfc

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
CLEANMASK = *.dlo *.lnk *.map *.obj *.o16 *.err *.log *.bak *.lib *.com *.sym *.bin *.exe *.dll *.wmp *.ppu *.rst *.res $(CLEAN_ADD)

!ifeq UNIX FALSE                 # Non-unix

!ifeq ENV OS2
COMSPEC   = $(OS_SHELL)          # Shell
OS2_SHELL = $(OS_SHELL)          #
RN  = @ren                       # Rename command
!else
!ifeq ENV Windows
RN  = move                       # Rename command
!else
!ifeq ENV WIN32
RN  = move                       # Rename command
!endif
!endif
!endif

SEP       = \                  # dir components separator
PS        = ;                  # paths separator
O         = obj                  # Object Extension differs from Linux to OS/2
O16       = o16                  # 16-bit obj
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
!else
!ifeq ENV WIN32
NULL      = nul
!else
NULL      = \dev\nul
!endif
!endif
BLACKHOLE = 2>&1 >$(NULL)
MKDIR     = @mkdir

EXE_SUFFIX = .exe
LIB_SUFFIX = .lib
LIB_PREFIX =
DLL_PREFIX =
DLL_SUFFIX = .dll

!else
!ifeq UNIX TRUE             # UNIX

SEP       = /                  # dir components separator
PS        = :                  # paths separator
O         = obj                  # Object Extension differs from Linux to OS/2
O16       = o16                  # 16-bit obj
DC        = rm -f                # Delete command is rm on linux and del on OS/2
CP        = cp                   # Copy command
RN        = mv                   # Rename command
SAY       = @echo                 # Echo message
MKBIN     = mkbin
GENHDD    = genhdd
GENFDD    = genfdd
FINDFILE  = findfile

if_not_exist_mkdir = ./if_not_exist_mkdir.sh

CLEAN_CMD    = @for %i in ($(CLEANMASK)) do @if exist $(PATH)%i $(DC) $(PATH)%i $(BLACKHOLE)

NULL      = /dev/null
BLACKHOLE = 2>&1 >$(NULL)
MKDIR     = mkdir

EXE_SUFFIX = l
LIB_SUFFIX = .a
LIB_PREFIX =
DLL_PREFIX =
DLL_SUFFIX = .so

!endif
!endif

MAPSYM    = @mapsym

LOG       = # >$(ROOT)$(SEP)compile.log 2>&1

!ifndef OBJS16
!ifdef  srcfiles16
p = $(PATH)
e = .$(O16)
OBJS16 = $+$(srcfiles16)$-
!endif
!endif

!ifndef OBJS
!ifdef  srcfiles
p = $(PATH)
e = .$(O)
OBJS = $+$(srcfiles)$-
!endif
!endif

SUF = $(SUF) .sym .exe .dll .lib .res .lnk .hlp .inf .o16 .obj .c16 .c .cpp .cc .asm .h .y .l .hpp .inc .rc .pas .pp .ipf .map .wmp .rexx .cmd

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

.o16: $(PATH)

.c:   $(PATH)

.c16: $(MYDIR)

.h:   $(PATH)

.hlp: $(PATH)

.inf: $(PATH)

.ipf: $(MYDIR)

.l.c: .AUTODEPEND
 $(DC) $^@
 lex -t $[@ >$^@

.y.c: .AUTODEPEND
 $(DC) $^*.h
 $(DC) $^*.c
 yacc.exe -y -d -o $^@ $[@

.c:   $(MYDIR)

.cpp: $(MYDIR)

.cc: $(MYDIR)

.h:   $(MYDIR)

.asm: $(MYDIR)

.pas: $(MYDIR)

.pp:  $(MYDIR)

.c16.o16: .AUTODEPEND
 $(SAY)  Compiling $[. $(LOG)
 $(CC16) $(C16OPT) $(COPT)   -fr=$^*.err -fo=$^@ $[@ $(LOG)

.c.obj: .AUTODEPEND
 $(SAY) Compiling $[. $(LOG)
 $(CC)  $(COPT)   -fr=$^*.err -fo=$^@ $[@ $(LOG)

.asm.obj: .AUTODEPEND
 $(SAY) Assembling $[. $(LOG)
 $(ASM) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

.cpp.obj: .AUTODEPEND
 $(SAY) Compiling $[. $(LOG)
 $(CPPC) $(COPT)  -fr=$^*.err -fo=$^@ $[@ $(LOG)

.cc.obj: .AUTODEPEND
 $(SAY) Compiling $[. $(LOG)
 $(CPPC) $(COPT)  -fr=$^*.err -fo=$^@ $[@ $(LOG)

.wmp.map: .AUTODEPEND
 $(SAY) Converting Watcom MAP to VAC MAP $[. $(LOG)
 $(AWK) -f $(FILESDIR)$(SEP)tools$(SEP)mapsym.awk <$< >$(PATH)$^@

.map.sym: .AUTODEPEND
 $(SAY) Converting VAC MAP to OS/2 SYM $[. $(LOG)
 $(MAPSYM) $[@
 $(RN) $^. $^:

.ipf.inf: .AUTODEPEND
 $(SAY) Compiling IPF source file $[.... $(LOG)
 $(HC) -i $[@ -o $^@

.ipf.hlp: .AUTODEPEND
 $(SAY) Compiling IPF source file $[.... $(LOG)
 $(HC) $[@ -o $^@

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

.lnk.dll: .symbolic
 $(SAY) Linking $^.... $(LOG)
 $(LINKER) $(LINKOPT) @$[@ $(LOG)

.rexx.exe: .AUTODEPEND
  $(SAY) Wrapping REXX code $[....
  rexxwrapper -program=$^* -rexxfiles=$^*.rexx -srcdir=$(%ROOT)$(SEP)tools$(SEP)rexxwrap -compiler=wcc -interpreter=os2rexx -intlib=rexx.lib -intincdir=$(%WATCOM)$(SEP)h$(SEP)os2 -compress

#
# "$(MAKE) subdirs" enters each dir in $(DIRS)
# and does $(MAKE) $(TARGET) in each dir:
#
subdirs: .symbolic
 @for %%i in ($(DIRS)) do cd $(MYDIR)%%i && $(MAKE) $(MAKEOPT) $(TARGET) && cd ..

dirhier: precopy .symbolic
 $(MDHIER) $(PATH)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
 $(CLEAN_CMD)

install: build
 $(SAY) Making install... $(LOG)
!ifeq INSTALL_PRE 1
 @$(MAKE) $(MAKEOPT) install_pre
!endif
!ifneq DEST
 $(MDHIER) $(DEST)
 @for %i in ($(TARGETS)) do @if exist %i $(CP) %i $(DEST)
!endif
!ifeq INSTALL_ADD 1
 @$(MAKE) $(MAKEOPT) install_add
!endif

precopy: .SYMBOLIC
 $(MAKE) $(MAKEOPT) -f $(ROOT)$(SEP)tools$(SEP)scripts$(SEP)makefile copy

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
