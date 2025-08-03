#
# OS/3 (osFree) project
# common make macros.
# (c) osFree project
# valerius, 2006/10/30
#

!ifndef __all_mk__
!define __all_mk__

all: precopy install .symbolic

!include $(%ROOT)tools/mk/dirs.mk
!include $(%ROOT)tools/mk/genrules.mk

print_vars: .symbolic
        @echo FILESDIR $(FILESDIR)
        @echo MYDIR    $(MYDIR) 
        @echo ROOT     $(ROOT)  
        @echo RELDIR   $(RELDIR)
        @echo PATH     $(PATH) 
        @echo CONTRIB  $(CONTRIB) 
        @echo PORT_BASE $(PORT_BASE)
        @echo BLD      $(BLD)
        @echo TOOLDIR  $(TOOLDIR)
        @echo DEST     $(DEST)
        @echo OS_REXX  $(OS_REXX)
        @echo OS_SHELL $(OS_SHELL)
        @echo ENV      $(ENV)
        @echo OS       $(%OS)
        @echo IMGDIR   $(IMGDIR)
        @echo IMGDIR1  $(IMGDIR1)
        @echo LIBDIR   $(LIBDIR)
        @echo LIBC     $(LIBC)
        @echo SEP      $(SEP)
        @echo TARGETS  $(TARGETS)
        @echo DEST     $(DEST)
        @echo LOGx     $(%LOG)
        @echo LOG      $(LOG)
        @echo LOG2     $(LOG2)

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

TRG  =

#
# Preprocessor defines
#
C_DEFS    = -zq -q -d__WATCOM__ -d__OSFREE__
!ifeq JWASM 1
ASM_DEFS  = -q -D__WATCOM__ -D__OSFREE__
!else
ASM_DEFS  = -zq -d__WATCOM__ -d__OSFREE__
!endif

#
# ADD_COPT and ADD_ASMOPT are defined in
# a file which includes this file.
#
# -q for quiet removes the credit from wcc386, wpp386 and wasm 
COPT      = $(C_DEFS) -q  &
            -i=$(MYDIR) -i=$(MYDIR).. &
            -i=$(PATH)  -i=$(PATH).. $(ADD_COPT)
COPT_LAST = $(DEFINES_LAST)

ASMOPT    = $(ASM_DEFS) $(ADD_ASMOPT) -q
C16OPT    = -nt=_TEXT16 -nd=D $(ADD_COPT)

# Watcom 1.7 RC has bug with resource storing. Resources not just added
# but replaced. So, we still use OS/2 TK RC.EXE
RC        = $(%INTERP)wrc -q
#RCOPT     = -bt=os2

RCOPT =  $(ADD_RCOPT)

#
# Tools:
#
# If you want to see the output from wcc... , wasm ... Remove the char '@' in front of the names.

!ifeq 32_BITS 1
# Removed @ from wpp386 wcc wpp wasm wlink
CC        = $(%INTERP)wcc386
CPPC      = $(%INTERP)wpp386
!else
CC        = $(%INTERP)wcc
CPPC      = $(%INTERP)wpp
!endif

CC16      = $(%INTERP)wcc
CPPC16    = $(%INTERP)wpp

!ifeq JWASM 1
ASM       = $(%INTERP)jwasm.exe
!else
ASM       = $(%INTERP)wasm
!endif

BINDER    = $(%INTERP)bind

LINKER    = $(%INTERP)wlink

!ifeq ENV WIN32
SC        = $(%INTERP)somc.exe
!else ifeq ENV WIN64
SC        = $(%INTERP)somc.exe
!else
SC        = $(%INTERP)sc.exe
!endif

# Note by valerius:
# don't add the following option to all.mk
# -----op internalrelocs----
# as it breaks all non-LX executables, for ex., bootsectors

!ifndef NOLIBS 1
LINKOPT   = $(ADD_LINKOPT) libpath $(%ROOT)$(SEP)build$(SEP)lib libpath $(%OS2TK)$(SEP)lib
!else
LINKOPT   = $(ADD_LINKOPT)
!endif

!ifeq %VERBOSE yes
LINKOPT = $(LINKOPT) op v
!else
LINKOPT = $(LINKOPT) op q
!endif

LIB       = $(%INTERP)wlib
LIBOPT    = -q -n -fo

# Don't add @ sign here. Will break build system
!ifdef %INTERP
MAKE      = `which qemu-i386` `which wmake`
!else
MAKE      = wmake
!endif

MAKEOPT   = -h

PC        = $(%INTERP)ppc386

#!ifeq %OS WIN64
#PC        = $(%INTERP)ppcx64
#!endif
!ifeq %ARCH amd64
PC        = $(%INTERP)ppcx64
!endif
PCOPT     = -v0 -l- -Sg2h $(ADD_PCOPT)

DD        = dd

SED       = $(%INTERP)sed
AWK       = $(%INTERP)awk
DOX       = doxygen

MC        = $(%INTERP)mkmsgf.exe
ME        = $(%INTERP)msgextrt.exe

HC        = $(%INTERP)wipfc

GENE2FS   = $(%INTERP)genext2fs

SYS       = sys

# A command to add a list of object
# files to a linker script
ADDFILES_CMD = @for %i in ($(OBJS)) do @%append $^@ FILE %i

#
# Extensions to clean up
#
CLEANMASK = *.dlo *.lnk *.map *.obj *.o16 *.mbr *.err *.log *.bak *.lib *.com *.sym *.bin *.exe *.dll *.wmp *.ppu *.rst *.res $(CLEAN_ADD)

!ifeq UNIX FALSE                 # Non-unix

!ifeq ENV OS2
%COMSPEC   = $(OS_SHELL)          # Shell
%OS2_SHELL = $(OS_SHELL)          #
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

!ifeq ENV Windows
NULL      = nul
CD        = cd /d
!else ifeq ENV WIN32
NULL      = nul
CD        = cd /d
!else ifeq ENV WIN64
NULL      = nul
CD        = cd /d
!else
NULL      = nul
CD        = $(REXX) cdir.cmd
!endif

MDHIER    = $(REXX) mdhier.cmd

!ifeq %VERBOSE yes
BLACKHOLE =
!else
BLACKHOLE = 2>&1 >$(NULL)
!endif
MKDIR     = $(verbose)mkdir

CLEAN_CMD    = $(verbose)echo for %i in ($(CLEANMASK)) do $(verbose)$(DC) $(PATH)%i $(BLACKHOLE)

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

NULL      = /dev/null
BLACKHOLE = 2>&1 >$(NULL)
MKDIR     = mkdir

CD        = cd
MDHIER    = mkdir -p

CLEAN_CMD    = @for %i in ($(CLEANMASK)) do $(verbose)$(DC) $(PATH)%i

EXE_SUF    = .exe
EXE_SUFFIX = .exe
LIB_SUFFIX = .lib
LIB_PREFIX =
DLL_PREFIX =
DLL_SUFFIX = .so

!endif

#DIR_PWD   = $+ $(%cwd) $-
#m  = $(MYDIR:$(SEP)$(SEP)=$(SEP))
#RELDIR       = $(MYDIR:$(ROOT)=)
#RELDIR_PWD       = $+  $(DIR_PWD:$(ROOT)=) $-
# This tells wmake to always read the current directory each time the variable DIR_PWD is read.
# Without it, the first time value stays the same (wmake caches it).
# $(%PWD)

MAPSYM    = @$(%INTERP)mapsym.exe

TOOLS     = $(ROOT)$(SEP)tools$(SEP)bin
!ifneq %LOG
LOG       = #>>$(BLD)build.txt
LOG2      = #2>&1
!else
LOG       = 
LOG2      = 
!endif
verbose=$(%VERBOSE)
!ifeq verbose yes
verbose=
!else
verbose=@
!endif

DEBUG=$(%DEBUG)
!ifeq DEBUG yes
DEBUG=all
!else
!undef DEBUG
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

SUF = $(SUF) .ico .sym .exe .com .dll .lib .res .rc .lnk .hlp .inf .o16 .obj .c16 .c .cpp .cc .asm .api .xh .ih .xih .h .hpp .inc .y .l .pas .pp .ipf .map .wmp .rexx .cmd .idl

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

.c:   $(SRC)

.h:   $(HDRS)

.h:   $(PATH)

.h:   $(SRC)

.xh:  $(PATH)

.ih:  $(PATH)

.xih: $(PATH)

.hlp: $(PATH)

.inf: $(PATH)

.ipf: $(MYDIR)

.l.c: .autodepend
 @$(SAY) LEX      $^. $(LOG)
!ifeq UNIX TRUE
 $(verbose)$(DC) $^@ $(BLACKHOLE)
!else
 $(verbose)if exist $^@ $(verbose)$(DC) $^@ $(BLACKHOLE)
!endif
!ifdef %INTERP
 $(verbose)`which qemu-i386` `which lex.exe` -t $[@ >$^@ $(LOG2)
!else
 $(verbose)lex.exe -t $[@ >$^@ $(LOG2)
!endif

# With -l yacc does not print "#line <nr>" in the generated C code.
.y.c: .autodepend
 @$(SAY) YACC     $^. $(LOG)
!ifeq UNIX TRUE
 $(verbose)$(DC) $^*.h $(BLACKHOLE)
 $(verbose)$(DC) $^*.c $(BLACKHOLE)
!else
 $(verbose)if exist $^*.h $(verbose)$(DC) $^*.h $(BLACKHOLE)
 $(verbose)if exist $^*.c $(verbose)$(DC) $^*.c $(BLACKHOLE)
!endif
!ifdef %INTERP
 $(verbose)`which qemu-i386` `which yacc.exe`  -y -d -o $^@ $[@ $(LOG2)
!else
 $(verbose)yacc.exe -y -d -o $^@ $[@ $(LOG2)
!endif

.y.h: .autodepend
 @$(SAY) YACC     $^. $(LOG)
!ifeq UNIX TRUE
 $(verbose)$(DC) $^*.h $(BLACKHOLE)
 $(verbose)$(DC) $^*.c $(BLACKHOLE)
!else
 $(verbose)if exist $^*.h $(verbose)$(DC) $^*.h $(BLACKHOLE)
 $(verbose)if exist $^*.c $(verbose)$(DC) $^*.c $(BLACKHOLE)
!endif
!ifdef %INTERP
 $(verbose)`which qemu-i386` `which yacc.exe` -y -d -o $^@ $[@ $(LOG2)
!else
 $(verbose)yacc.exe -y -d -o $^@ $[@ $(LOG2)
!endif

.c:   $(MYDIR)

.cpp: $(MYDIR)

.cc:  $(MYDIR)

.h:   $(MYDIR)

.idl: $(MYDIR)

.asm: $(MYDIR)

.pas: $(MYDIR)

.pp:  $(MYDIR)

.c16.o16: .AUTODEPEND
 @$(SAY) CC16     $^. $(LOG)
 $(verbose)$(CC16) $(C16OPT) $(COPT)   -fr=$^*.err -fo=$^@ $[@ $(LOG2)

.c.obj: .AUTODEPEND
 @$(SAY) CC       $^. $(LOG)
 $(verbose)$(CC)  $(COPT)   -fr=$^*.err -fo=$^@ $[@ $(LOG2)

.asm.obj: .AUTODEPEND
 @$(SAY) ASM      $^. $(LOG)
!ifeq JWASM 1
 $(verbose)$(ASM) $(ASMOPT) -Fw=$^*.err -Fo=$^@ $[@ $(LOG2)
!else
 $(verbose)$(ASM) $(ASMOPT) -fr=$^*.err -fo=$^@ $[@ $(LOG2)
!endif

.cpp.obj: .AUTODEPEND
 @$(SAY) CXX      $^. $(LOG)
 $(verbose)$(CPPC) $(COPT)  -fr=$^*.err -fo=$^@ $[@ $(LOG2)

.cc.obj: .AUTODEPEND
 @$(SAY) CXX      $^. $(LOG)
 $(verbose)$(CPPC) $(COPT)  -fr=$^*.err -fo=$^@ $[@ $(LOG2)

.wmp.map: .AUTODEPEND
 @$(SAY) WMP2MAP  $^. $(LOG)
 $(verbose)$(AWK) -f $(FILESDIR)$(SEP)host$(SEP)bin$(SEP)mapsym.awk <$< >$(PATH)$^@ $(LOG2)

.map.sym: .AUTODEPEND
 @$(SAY) MAPSYM   $^. $(LOG)
 $(verbose)$(MAPSYM) $[@ $(LOG2)
 $(verbose)$(RN) $^. $^: $(LOG2)

.ipf.inf: .AUTODEPEND
 @$(SAY) IPFC     $^. $(LOG)
 $(verbose)$(HC) -i $[@ -o $^@ $(LOG2)

.ipf.hlp: .AUTODEPEND
 @$(SAY) IPFC     $^. $(LOG)
 $(verbose)$(HC) $[@ -o $^@ $(LOG2)

.pas.exe: .symbolic
 @$(SAY) PPC      $^. $(LOG)
 $(verbose)$(PC) $(PCOPT) -o$^. -FE$^: -Fe$^: $[@ $(LOG2)

.pp.exe: .symbolic
 @$(SAY) PPC      $^. $(LOG)
 $(verbose)$(PC) $(PCOPT) -o$^. -FE$^: -Fe$^: $[@ $(LOG2)

.lnk.exe: .autodepend
 @$(SAY) LINK     $^. $(LOG)
 $(verbose)$(LINKER) $(LINKOPT) @$[@ $(LOG2)

.lnk.com: .autodepend
 @$(SAY) LINK     $^. $(LOG)
 $(verbose)$(LINKER) $(LINKOPT) @$[@ $(LOG2)

.lnk.dll: .autodepend
 @$(SAY) LINK     $^. $(LOG)
 $(verbose)$(LINKER) $(LINKOPT) @$[@ $(LOG2)

.rexx.exe: .AUTODEPEND
 @$(SAY) WRAPXX   $^. $(LOG)
 $(verbose)$(%INTERP)rexxwrapper -program=$^* -rexxfiles=$^*.rexx -srcdir=$(%ROOT)$(SEP)tools$(SEP)rexxwrap -compiler=wcc -interpreter=os2rexx -intlib=rexx.lib -intincdir=$(%WATCOM)$(SEP)h$(SEP)os2 -compress $(LOG2)

.idl.ih: .AUTODEPEND
 @$(SAY) SC       $^. $(LOG)
 $(verbose)$(%INTERP)$(%OS2TK)$(SEP)som$(SEP)bin$(SEP)sc.exe -sih $< -o $^: $(ADD_SCOPT)

.idl.xih: .AUTODEPEND
 @$(SAY) SC       $^. $(LOG)
 $(verbose)$(%INTERP)$(%OS2TK)$(SEP)som$(SEP)bin$(SEP)sc.exe -sxih $< -o $^: $(ADD_SCOPT)

.idl.xh: .AUTODEPEND
 @$(SAY) SC       $^. $(LOG)
 $(verbose)$(%INTERP)$(%OS2TK)$(SEP)som$(SEP)bin$(SEP)sc.exe -sxh $< -o $^: $(ADD_SCOPT)

.idl.h: .AUTODEPEND
 @$(SAY) SC       $^. $(LOG)
 $(verbose)$(%INTERP)$(%OS2TK)$(SEP)som$(SEP)bin$(SEP)sc.exe -sh $< -o $^: $(ADD_SCOPT)

.idl.api: .AUTODEPEND
 @$(SAY) SC       $^. $(LOG)
 $(verbose)$(%INTERP)$(%OS2TK)$(SEP)som$(SEP)bin$(SEP)sc.exe -sapi $< -o $^: $(ADD_SCOPT)

#
# "$(MAKE) subdirs" enters each dir in $(DIRS)
# and does $(MAKE) $(TARGET) in each dir:
#

TARGET = install

subdirs: .symbolic
!ifeq UNIX TRUE
 @for %d in ($(DIRS)) do @if [ -d $(MYDIR)%d ]; then cd $(MYDIR)%d && $(MAKE) $(MAKEOPT) $(TARGET) && cd ..; fi
!else
 @for %d in ($(DIRS)) do @if exist $(MYDIR)%d $(CD) $(MYDIR)%d && $(MAKE) $(MAKEOPT) $(TARGET)
!endif

dirhier: .symbolic
 $(verbose)$(SAY) CD       $(RELDIR) $(LOG)
 $(verbose)$(MDHIER) $(PATH)

clean: .symbolic
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
 $(verbose)$(CLEAN_CMD)

prepall: .symbolic
 @$(MAKE) $(MAKEOPT) prep
 @$(MAKE) $(MAKEOPT) TARGET=prepall subdirs

targets: prep prereq subdirs .symbolic
 @for %t in ($(TARGETS)) do @$(MAKE) -f $(mf) $(MAKEOPT) %t

!ifdef PROJ
$(PATH)$(PROJ).lnk: $(OBJS) $(MYDIR)makefile
!endif

!ifeq  DEST $(PATH)

FLG  =

!else ifdef DEST

!ifdef TRGT
FLG  = $(TRGT)
!else
FLG  = $(TARGETS:$(PATH)=)
!endif

!endif

!ifdef INSTALL
FLG  = install2
!endif

install: targets $(DEST)$(SEP)$(FLG) .symbolic
!ifdef INSTALL_ADD 1
 @$(MAKE) $(MAKEOPT) install_add
!endif

$(PATH)subdirs: .symbolic

!ifneq TRGT ""
$(PATH)$(FLG): .symbolic
!endif

!ifndef TRGT
install3: $(PATH)$(file) .symbolic
!else
install3: .symbolic
!endif
!ifneq DEST
!ifneq DEST $(PATH)
!ifneq file subdirs
 @$(SAY) INST     $(file) $(LOG)
 @$(MDHIER) $(DEST) $(LOG2)
 $(verbose) $(CP) $(PATH)$(file) $(DEST)$(SEP)$(file) $(BLACKHOLE)
!endif
!endif
!endif

!ifdef INSTALL

$(DEST)$(SEP)install2: .symbolic
 @for %i in ($(INSTALL)) do @$(MAKE) $(MAKEOPT) file=%i install3

!else

$(DEST)$(SEP)$(FLG): .symbolic
 @for %i in ($(FLG)) do @$(MAKE) $(MAKEOPT) file=%i install3

!endif

precopy: .symbolic
 @$(MAKE) $(MAKEOPT) -f $(ROOT)tools$(SEP)scripts$(SEP)makefile tools

# prebuild libs
prelibs: .symbolic
 @if exist $(FILESDIR)libs-built-flag.flg @%quit
!ifdef __appsos2_mk__
 # build all_shared.lib
 $(verbose)cd $(ROOT)OS2$(SEP)Shared && $(MAKE) $(MAKEOPT) install
!endif
!ifdef __apps_os2_cmd_mk__
 # build cmd_shared.lib
 $(verbose)cd $(ROOT)OS2$(SEP)CMD$(SEP)shared && $(MAKE) $(MAKEOPT) install
!endif
!ifdef __apps_os2_pm_mk__
 # build pm_shared.lib
 $(verbose)cd $(ROOT)OS2$(SEP)PM$(SEP)shared && $(MAKE) $(MAKEOPT) install
!endif
 @$(%INTERP)wtouch $(FILESDIR)libs-built-flag.flg
 $(verbose)cd $(MYDIR)

!ifndef WRAPPERS

# define if gen_compile_rules_wrapper and gen_deps_wrapper
# are not defined
gen_compile_rules_wrapper: .symbolic
 @%null

gen_deps_wrapper: .symbolic
 @%null

!endif

!ifndef __port_mk__
prep: .symbolic
!endif

!ifdef DEBUG
ADD_LINKOPT = DEBUG $(DEBUG) $(ADD_LINKOPT)
!endif

!endif
