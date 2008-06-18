#
# OS/3 (osFree) boot sequence project
# common make macros.
# (c) osFree project
# valerius, 2006/10/30
#

ROOT=$(%ROOT)

#
# Preprocessor defines
#
C_DEFS    = -zq #-d__OS2__ -d__WATCOM__
ASM_DEFS  = -zq #-d__OS2__ -d__WATCOM__

#
# ADD_COPT and ADD_ASMOPT are defined in
# a file which includes this file.
#
!ifdef 32_BITS
COPT      = $(C_DEFS) -i=. -i=.. $(ADD_COPT)
ASMOPT    = $(ASM_DEFS)  $(ADD_ASMOPT)
!else
COPT      = -ms $(C_DEFS) -i=. -i=.. $(ADD_COPT)
ASMOPT    = -bt=DOS -ms $(ASM_DEFS)  $(ADD_ASMOPT)
!endif

#
# Tools:
#
!ifdef 32_BITS
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
LIBOPT    = -q

# Don't add @ sign here. Will break build system
MAKE      = wmake
MAKEOPT   = -h

PC        = ppc386
PCOPT     = -Sg2h

SED       = sed
AWK       = awk
DOX       = doxygen

DD        = dd

GENE2FS    = genext2fs
SYS       = sys

#.SUFFIXES: .o

# A command to add a list of object
# files to a linker script
ADDFILES_CMD = @for %%i in ($(OBJS)) do @%append $^&.lnk FILE %%i

#
# Extensions to clean up
#
CLEANMASK = *.lnk *.map *.obj *.err *.log *.bak *.lib *.com *.sym *.bin *.exe *.wmp

!ifeq UNIX FALSE                 # Non-unix

!ifeq ENV OS/2
COMSPEC   = $(OS_SHELL)          # Shell
OS2_SHELL = $(OS_SHELL)          #
RN  = @move                      # Rename command
!else ifeq ENV Windows
RN  = @ren                       # Rename command
!endif

TOOLS     = $(ROOT)\tools\bin
LOG       = # 2>&1 >> $(ROOT)\compile.log
SEP       = \                    # dir components separator
PS        = ;                    # paths separator
O         = obj                  # Object Extension differs from Linux to OS/2
DC        = @del                 # Delete command is rm on linux and del on OS/2
CP        = @copy                # Copy command
SAY       = @echo                # Echo message

MKBIN     = $(REXX) mkbin.cmd
GENHDD    = $(REXX) genhdd.cmd
GENFDD    = $(REXX) genfdd.cmd
FINDFILE  = $(REXX) findfile.cmd
BUILD     = $(TOOLS)\build.exe

CLEAN_CMD    = @for %i in ($(CLEANMASK)) do @if exist %i $(DC) %i $(BLACKHOLE)

!ifeq ENV Windows
NULL      = nul
!else
NULL      = \dev\nul
!endif
BLACKHOLE = >$(NULL) 2>&1

!else ifeq UNIX TRUE             # UNIX

TOOLS     = $(ROOT)/tools/bin
LOG       = # 2>&1 >> $(ROOT)/compile.log
SEP       = /                    # dir components separator
PS        = :                    # paths separator
O         = obj                  # Object Extension differs from Linux to OS/2
DC        = rm -f                # Delete command is rm on linux and del on OS/2
CP        = cp                   # Copy command
RN        = mv                   # Rename command
SAY       = echo                 # Echo message
MKBIN     = mkbin
GENHDD    = genhdd
GENFDD    = genfdd
FINDFILE  = findfile
BUILD     = $(TOOLS)/build

CLEAN_CMD    = @for %i in ($(CLEANMASK)) do @if exist %i $(DC) %i $(BLACKHOLE)

NULL      = /dev/null
BLACKHOLE = >$(NULL) 2>&1

!endif

.SUFFIXES:
.SUFFIXES:  .sym .xfd .fsd .bin .exe .com .dll .lib .$(O) .res .inf .c .cpp .asm .h .hpp .inc .rc .pas .ipf .map .wmp .rexx

.c.$(O): .AUTODEPEND
 $(SAY) Compiling $< $(LOG)
 $(CC) $(COPT) -fo=$^&.$(O) $< $(LOG)

.asm.$(O): .AUTODEPEND
 $(SAY) Assembling $< $(LOG)
 $(ASM) $(ASMOPT) -fo=$^&.$(O) $< $(LOG)

#
# "$(MAKE) subdirs" enters each dir in $(DIRS)
# and does $(MAKE) $(TARGET) in each dir:
#
subdirs: .SYMBOLIC
 @for %%i in ($(DIRS)) do @ cd %%i && cd && $(MAKE) $(MAKEOPT) $(TARGET) && cd ..
