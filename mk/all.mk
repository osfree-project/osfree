#
# OS/3 (osFree) project
# common make macros.
# (c) osFree project
# valerius, 2006/10/30
#

!ifndef __all_mk__
!define __all_mk__

all: targets .symbolic # precopy prereq
 @%null

# a double (see the end of file )
#install: build

!include $(%ROOT)/mk/dirs.mk
!include $(%ROOT)/mk/genrules.mk

#print_vars: .SYMBOLIC
#        @echo FILESDIR $(FILESDIR)
#        @echo MYDIR    $(MYDIR) 
#        @echo ROOT     $(ROOT)  
#        @echo RELDIR   $(RELDIR)
#        @echo PATH     $(PATH) 
#        @echo BLD      $(BLD)
#        @echo RELDIR   $(RELDIR)
#        @echo TOOLDIR  $(TOOLDIR)
#        @echo DEST     $(DEST)
#        @echo x        $(x)
#        @echo q        $(q)
#        @echo OS_REXX  $(OS_REXX)
#        @echo OS_SHELL $(OS_SHELL)
#        @echo ENV      $(ENV)
#        @echo IMGDIR   $(IMGDIR)
#        @echo IMGDIR1  $(IMGDIR1)
#        @echo LIBDIR   $(LIBDIR)
#        @echo LIBC     $(LIBC)
#        @echo SEP      $(SEP)
#        @echo TARGETS  $(TARGETS)
#        @echo DEST     $(DEST)

#prevshared: .symbolic
# @if not exist $(MYDIR)..$(SEP)..$(SEP)Shared @%quit
# $(CD) $(MYDIR)..$(SEP)..$(SEP)Shared
# @$(MAKE) $(MAKEOPT)
# $(CD) $(MYDIR)
#
# make 'shared' lib on this level
# before making any programs
#shared: prevshared .symbolic
# @if not exist $(MYDIR)..$(SEP)Shared @%quit
# @cd $(MYDIR)..$(SEP)Shared
# @$(MAKE) $(MAKEOPT)
# $(CD) $(MYDIR)

# build and install each target in sequence
build: precopy prereq .SYMBOLIC
 @for %t in ($(TARGETS)) do @$(MAKE) $(MAKEOPT) -f $(mf) %t

TRG  =

#
# Preprocessor defines
#
C_DEFS    = -zq -q -d__WATCOM__ -d__OSFREE__
ASM_DEFS  = -zq -d__WATCOM__ -d__OSFREE__

#
# ADD_COPT and ADD_ASMOPT are defined in
# a file which includes this file.
#
# -q for quiet removes the credit from wcc386, wpp386 and wasm 
COPT      = $(C_DEFS) -q &
            -i=$(MYDIR) -i=$(MYDIR).. &
            -i=$(PATH)  -i=$(PATH).. $(ADD_COPT)
COPT_LAST = $(DEFINES_LAST)

#            -i=$(%WATCOM)$(SEP)h$(SEP)os2 # until UniAPI headers will be ready
ASMOPT    = $(ASM_DEFS) $(ADD_ASMOPT) -q
C16OPT    = -nt=_TEXT16 -nd=D $(ADD_COPT)

# Watcom 1.7 RC has bug with resource storing. Resources not just added
# but replaced. So, we still use OS/2 TK RC.EXE
RC        = @wrc -q
#RCOPT     = -bt=os2

RCOPT =  $(ADD_RCOPT)

#
# Tools:
#
# If you want to see the output from wcc... , wasm ... Remove the char '@' in front of the names.

!ifeq 32_BITS 1
# Removed @ from wpp386 wcc wpp wasm wlink
CC        = wcc386
CPPC      = wpp386
!else
CC        = wcc
CPPC      = wpp
!endif

CC16      = wcc
CPPC16    = wpp

ASM       = wasm

LINKER    = wlink
# Note by valerius:
# don't add the following option to all.mk
# -----op internalrelocs----
# as it breaks all non-LX executables, for ex., bootsectors

!ifndef NOLIBS 1
LINKOPT   = op q libpath $(%ROOT)$(SEP)build$(SEP)lib $(ADD_LINKOPT)
!else
LINKOPT   = op q $(ADD_LINKOPT)
!endif

LIB       = wlib
LIBOPT    = -q -n -fo

# Don't add @ sign here. Will break build system
MAKE      = wmake
MAKEOPT   = -h

PC        = ppc386
PCOPT     = -Sg2h $(ADD_PCOPT)

DD        = dd

SED       = sed
AWK       = awk
DOX       = doxygen

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
RN  = move                       # Rename command
!else ifeq ENV Windows
RN  = move                       # Rename command
!else ifeq ENV WIN32
RN  = move                       # Rename command
!endif

SEP       = \                    # dir components separator
PS        = ;                    # paths separator
O         = obj                  # Object Extension differs from Linux to OS/2
O16       = o16                  # 16-bit obj
DC        = del                 # Delete command is rm on linux and del on OS/2
CP        = copy                # Copy command
SAY       = echo                # Echo message
MKBIN     = $(REXX) mkbin.cmd
GENHDD    = $(REXX) genhdd.cmd
GENFDD    = $(REXX) genfdd.cmd
FINDFILE  = $(REXX) findfile.cmd
if_not_exist_mkdir = if_not_exist_mkdir.cmd

!ifeq ENV Windows
NULL      = nul
!else ifeq ENV WIN32
NULL      = nul
!else
NULL      = nul
!endif

BLACKHOLE = 2>&1 >$(NULL)
MKDIR     = @mkdir

CLEAN_CMD    = @echo for %i in ($(CLEANMASK)) do @if exist $(PATH)%i @$(DC) $(PATH)%i $(BLACKHOLE)

EXE_SUF    = .exe
EXE_SUFFIX = .exe
LIB_SUFFIX = .lib
LIB_PREFIX =
DLL_PREFIX =
DLL_SUFFIX = .dll

!else ifeq UNIX TRUE                  # UNIX

SEP       = /                    # dir components separator
PS        = :                    # paths separator
O         = obj                  # Object Extension differs from Linux to OS/2
O16       = o16                  # 16-bit obj
DC        = rm -f                # Delete command is rm on linux and del on OS/2
CP        = cp                   # Copy command
RN        = mv                   # Rename command
SAY       = echo                 # Echo message
MKBIN     = mkbin
GENHDD    = genhdd
GENFDD    = genfdd
FINDFILE  = findfile

if_not_exist_mkdir = $(ROOT)$(SEP)if_not_exist_mkdir.sh

NULL      = /dev/null
BLACKHOLE = 2>&1 >$(NULL)
MKDIR     = mkdir

CLEAN_CMD    = @for %i in ($(CLEANMASK)) do @$(DC) $(PATH)%i

EXE_SUF    =
EXE_SUFFIX = l
LIB_SUFFIX = .lib
LIB_PREFIX =
DLL_PREFIX =
DLL_SUFFIX = .so

!endif

DIR_PWD   = $+ $(%cwd) $-
#RELDIR       = $(MYDIR:$(ROOT)=)
RELDIR_PWD       = $+  $(DIR_PWD:$(ROOT)=) $-
# This tells wmake to always read the current directory each time the variable DIR_PWD is read.
# Without it, the first time value stays the same (wmake caches it).
# $(%PWD)

MAPSYM    = @mapsym

TOOLS     = $(ROOT)$(SEP)tools$(SEP)bin
LOG       = # >$(ROOT)$(SEP)compile.log 2>&1
            # 2>&1 >> $(ROOT)$(SEP)compile.log
            # Alternative log command (use manually on cmd line, for bash):
            #   2>&1 | tee ${ROOT}${SEP}compile.log
verbose=$(%VERBOSE)
!ifeq verbose yes
verbose=
!else
verbose=@
!endif

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

SUF = $(SUF) .sym .exe .dll .lib .res .rc .lnk .hlp .inf .o16 .obj .c16 .c .cpp .cc .asm .h .hpp .inc .y .l .pas .pp .ipf .map .wmp .rexx .cmd

.SUFFIXES:
.SUFFIXES: $(SUF)

# On Linux with Bash %PWD prints out the working directory
# Can wmake's macro do the same?  $^:  or this  $]:
#  Wmake has similar stuff in the variable &(%cwd)

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

.c16: $(PATH)

.h:   $(PATH)

.hlp: $(PATH)

.inf: $(PATH)

.ipf: $(MYDIR)

.l.c: .autodepend
 @$(SAY) LEX      $^. $(LOG)
!ifeq UNIX TRUE
 @$(DC) $^@ $(BLACKHOLE)
!else
 @if exist $^@ @$(DC) $^@ $(BLACKHOLE)
!endif
 $(verbose)lex -t $[@ >$^@ $(LOG)

# With -l yacc does not print "#line <nr>" in the generated C code.
.y.c: .autodepend
 @$(SAY) YACC     $^. $(LOG)
!ifeq UNIX TRUE
 @(verbose)@$(DC) $^*.h $(BLACKHOLE)
 @$(DC) $^*.c $(BLACKHOLE)
!else
 @if exist $^*.h @$(DC) $^*.h $(BLACKHOLE)
 @if exist $^*.c @$(DC) $^*.c $(BLACKHOLE)
!endif
 $(verbose)yacc -y -d -o $^@ $[@ $(LOG)

.c:   $(MYDIR)

.cpp: $(MYDIR)

.cc: $(MYDIR)

.h:   $(MYDIR)

.asm: $(MYDIR)

.pas: $(MYDIR)

.pp:  $(MYDIR)

.c16.o16: .AUTODEPEND
 @$(SAY) CC16     $^. $(LOG)
 $(verbose)$(CC16) $(C16OPT) $(COPT)   -fr=$^*.err -fo=$^@ $[@ $(LOG)

.c.obj: .AUTODEPEND
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC)  $(COPT)   -fr=$^*.err -fo=$^@ $[@ $(LOG)

.asm.obj: .AUTODEPEND
 @$(SAY) ASM      $^. $(LOG)
 $(verbose)$(ASM) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG)

.cpp.obj: .AUTODEPEND
 @$(SAY) CXX      $^. $(LOG)
 $(verbose)$(CPPC) $(COPT)  -fr=$^*.err -fo=$^@ $[@ $(LOG)

.cc.obj: .AUTODEPEND
 @$(SAY) CXX      $^. $(LOG)
 $(verbose)$(CPPC) $(COPT)  -fr=$^*.err -fo=$^@ $[@ $(LOG)

.wmp.map: .AUTODEPEND
 @$(SAY) WMP2MAP  $^. $(LOG)
 $(verbose)$(AWK) -f $(FILESDIR)$(SEP)tools$(SEP)mapsym.awk <$< >$(PATH)$^@ $(LOG)

.map.sym: .AUTODEPEND
 @$(SAY) MAPSYM   $^. $(LOG)
 $(verbose)$(MAPSYM) $[@ $(LOG)
 @$(RN) $^. $^: $(LOG)

.ipf.inf: .AUTODEPEND
 @$(SAY) IPFC     $^. $(LOG)
 $(verbose)$(HC) -i $[@ -o $^@ $(BLACKHOLE)

.ipf.hlp: .AUTODEPEND
 @$(SAY) IPFC     $^. $(LOG)
 $(verbose)$(HC) $[@ -o $^@ $(BLACKHOLE)

.pas.exe: .symbolic
 @$(SAY) PPC      $^. $(LOG)
 $(verbose)$(PC) $(PCOPT) -o$^. -FE$^: -Fe$^: $[@ $(BLACKHOLE)

.pp.exe: .symbolic
 @$(SAY) PPC      $^. $(LOG)
 $(verbose)$(PC) $(PCOPT) -o$^. -FE$^: -Fe$^: $[@ $(BLACKHOLE)

.lnk.exe: .autodepend
 @$(SAY) LINK     $^. $(LOG)
 $(verbose)$(LINKER) $(LINKOPT) @$[@ $(LOG)

.lnk.dll: .autodepend
 @$(SAY) LINK     $^. $(LOG)
 $(verbose)$(LINKER) $(LINKOPT) @$[@ $(LOG)

.rexx.exe: .AUTODEPEND
 @$(SAY) WRAPXX   $^. $(LOG)
 $(verbose)rexxwrapper -program=$^* -rexxfiles=$^*.rexx -srcdir=$(%ROOT)$(SEP)tools$(SEP)rexxwrap -compiler=wcc -interpreter=os2rexx -intlib=rexx.lib -intincdir=$(%WATCOM)$(SEP)h$(SEP)os2 -compress $(LOG)

targets: prereq subdirs install .symbolic
 @%null
 #@for %t in ($(TARGETS) install) do @$(MAKE) $(MAKEOPT) %t

#
# "$(MAKE) subdirs" enters each dir in $(DIRS)
# and does $(MAKE) $(TARGET) in each dir:
#
subdirs: .symbolic
# @for %%i in ($(DIRS)) do @cd $(MYDIR)%%i && $(SAY) cd $(DIR_PWD)$(SEP)%%i && $(MAKE) $(MAKEOPT) $(TARGET) && cd ..
# @for %%i in ($(DIRS)) do  @(@cd $(MYDIR)%%i && @$(MAKE) $(MAKEOPT) $(TARGET) && cd ..)
# @for %i in ($(DIRS)) do  @if exist $(MYDIR)%i @cd $(MYDIR)%i && @$(MAKE) $(MAKEOPT) $(TARGET)
 @for %d in ($(DIRS)) do @if exist $(MYDIR)%d @cd $(MYDIR)%d && @$(MAKE) $(MAKEOPT) targets

dirhier: precopy .symbolic
 # @$(SAY) MKDIR    $(PATH) $(LOG)
 $(verbose)$(MDHIER) $(PATH) $(LOG)

.IGNORE
clean: .SYMBOLIC
 # @$(SAY) CLEAN    $(LOG)
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
 $(verbose)$(CLEAN_CMD)
 # @$(SAY)   "$(TARGETS) -> $(DEST)"

#$(PATH)$(PROJ)$(EXE_SUF):

#$(PATH)$(PROJ).com:

#$(PATH)$(PROJ):

$(PATH)$(PROJ).lnk: $(MYDIR)makefile

!ifndef TRGT
!ifdef INSTALL
FLG  = install2
!else
FLG  = 
!endif
!else ifneq DEST none
FLG  = $(DEST)$(SEP)$(TRGT)
!else
FLG  = $(PATH)$(TRGT)
!endif

install: build $(FLG) .symbolic # $(TARGETS) $(DEST)$(SEP)$(TRGT)
!ifeq INSTALL_ADD 1
 @$(MAKE) $(MAKEOPT) install_add
!endif

install2: .symbolic
 @for %i in ($(INSTALL)) do @$(MAKE) $(MAKEOPT) file=%i install3

$(DEST)$(SEP)$(TRGT): $(PATH)$(TRGT)
 @$(MAKE) $(MAKEOPT) file=$(TRGT) install3

install3: .symbolic
 @if exist $(DEST)$(SEP)$(file) @%quit
 @$(SAY) INST     $(file) $(LOG)
 @$(MDHIER) $(DEST)
 @$(CP) $(PATH)$(file) $(DEST)$(SEP)$(file) $(BLACKHOLE)

#install: build .symbolic
#!ifeq PROJ
# %null
#!else
# @$(SAY) INST     $(PROJ) $(LOG)
#!ifeq INSTALL_PRE 1
# @$(MAKE) $(MAKEOPT) install_pre
#!endif
#!ifneq DEST
# #-$(MDHIER) $(DEST)
# @if not $(DEST) == none $(verbose)$(MDHIER) $(DEST)
# @for %i in ($(TARGETS)) do @if exist %i @if not $(DEST) == none @$(CP) %i $(DEST) $(BLACKHOLE)
#!endif
#!ifeq INSTALL_ADD 1
# @$(MAKE) $(MAKEOPT) install_add
#!endif
# @wtouch $(PATH)$(PROJ).flg
#!endif

precopy: .SYMBOLIC
 # @$(SAY) PRECOPY  scrpits $(LOG)
 @$(MAKE) $(MAKEOPT) -f $(%ROOT)$(SEP)tools$(SEP)scripts$(SEP)makefile tools

# $(FILESDIR)$(SEP)libs-built-flag: prelibs .symbolic

# prebuild libs
prelibs: .symbolic
 #!ifdef __appsos2_mk__
 # build all_shared.lib
 #@$(SAY) cd $(%ROOT)OS2$(SEP)Shared
 @$(CD) $(%ROOT)OS2$(SEP)Shared && @$(MAKE) $(MAKEOPT) all
 #!endif
 #!ifdef __apps_os2_cmd_mk__
 # build cmd_shared.lib
 #@$(SAY) cd $(%ROOT)OS2$(SEP)cmd$(SEP)Shared
 @$(CD) $(%ROOT)OS2$(SEP)CMD$(SEP)Shared && @$(MAKE) $(MAKEOPT) all
 #!endif
 #!ifdef __apps_os2_pm_mk__
 # build pm_shared.lib
 @$(SAY) cd $(%ROOT)OS2$(SEP)PM$(SEP)Shared
 @$(CD) $(%ROOT)OS2$(SEP)PM$(SEP)Shared && @$(MAKE) $(MAKEOPT) all
 #!endif
 # @touch $(FILESDIR)$(SEP)libs-built-flag
 @$(CD) $(MYDIR)

.error:
  %abort

test:
 @echo $(TRGT)

!ifndef WRAPPERS

# define if gen_compile_rules_wrapper and gen_deps_wrapper
# are not defined
gen_compile_rules_wrapper: .SYMBOLIC
 @%null

gen_deps_wrapper: .SYMBOLIC
 @%null

!endif

!endif
