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
C_DEFS    = -q #-d__OS2__ -d__WATCOM__
ASM_DEFS  = #-d__OS2__ -d__WATCOM__

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
CC        = wcc386
!else
CC        = wcc
!endif

ASM       = wasm
LINKER    = wlink
LIB       = wlib

MAKE      = wmake
MAKEOPT   = -h

PC       = ppc386

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
MKBIN     = mkbin.cmd
GENHDD    = genhdd.cmd
GENFDD    = genfdd.cmd
FINDFILE  = findfile.cmd
BUILD     = $(TOOLS)\build.exe

NULL      = \dev\nul
BLACKHOLE = >$(NULL) 2>&1

CLEAN_CMD    = @for %%i in ($(CLEANMASK)) do $(DC) %%i $(BLACKHOLE)

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

NULL      = /dev/null
BLACKHOLE = >$(NULL) 2>&1

CLEAN_CMD    = $(DC) $(CLEANMASK) $(BLACKHOLE)

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
