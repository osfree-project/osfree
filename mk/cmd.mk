#
# OS/3 (osFree) boot sequence project
# common make macros.
# (c) osFree project
# valerius, 2006/10/30
#

#
# Preprocessor defines
#
C_DEFS    = -d__OS2__ -d__WATCOM__
ASM_DEFS  = -d__OS2__ -d__WATCOM__

#
# ADD_COPT and ADD_ASMOPT are defined in
# a file which includes this file.
#
!ifdef 32_BITS
COPT      = $(C_DEFS) -i=$(ROOT)\include\os3 -i=. -i=.. $(ADD_COPT)
ASMOPT    = -bt=DOS $(ASM_DEFS)  $(ADD_ASMOPT)
!else
COPT      = -ms $(C_DEFS) -i=$(ROOT)\include\os3 -i=. -i=.. $(ADD_COPT)
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

PC        = ppc386
PCOPT     = -n -FE$(DIR) -Fu$(ROOT)$(SEP)src$(SEP)toolkit$(SEP)units -Fl$(ROOT)$(SEP)src$(SEP)toolkit$(SEP)units -Fi$(ROOT)$(SEP)src$(SEP)toolkit$(SEP)units -FU.

LINKOPT   = lib $(ROOT)$(SEP)lib$(SEP)cmd_shared.lib lib $(ROOT)$(SEP)lib$(SEP)all_shared.lib
LINKLIB   = $(ADD_LIB)
LIBPATH   = $(%WATCOM)$(SEP)lib386;$(%WATCOM)$(SEP)lib386$(SEP)os2;$(%WATCOM)$(SEP)lib286;$(%WATCOM)$(SEP)lib286$(SEP)os2


HC        = ipfc

RC        = rc
RCOPT=

MC        = mkmsgf
MAPSYM    = mapsym

SED       = sed
AWK       = awk
DOX       = doxygen

GENE2FS   = genext2fs
SYS       = sys

# A command to add a list of object
# files to a linker script
ADDFILES_CMD = @for %%i in ($(OBJS)) do @%append $^&.lnk FILE %%i

#
# Extensions to clean up
#
CLEANMASK = *.lnk *.map *.obj *.err *.log *.bak *.lib *.com *.sym *.bin

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
O         = obj                  # Object Extension differs from Linux to OS/2
DC        = @del                 # Delete command is rm on linux and del on OS/2
CP        = @copy                # Copy command
SAY       = @echo                # Echo message
MKBIN     = mkbin.cmd
GENHDD    = genhdd.cmd
GENFDD    = genfdd.cmd
FINDFILE  = findfile.cmd

NULL      = \dev\nul
BLACKHOLE = >$(NULL) 2>&1

CLEAN_CMD    = @for %%i in ($(CLEANMASK)) do $(DC) %%i $(BLACKHOLE)

!else ifeq UNIX TRUE             # UNIX

TOOLS     = $(ROOT)/tools/bin
LOG       = # 2>&1 >> $(ROOT)/compile.log
O         = obj                  # Object Extension differs from Linux to OS/2
DC        = rm -f                # Delete command is rm on linux and del on OS/2
CP        = cp                   # Copy command
RN        = mv                   # Rename command
SAY       = echo                 # Echo message
MKBIN     = mkbin
GENHDD    = genhdd
GENFDD    = genfdd
FINDFILE  = findfile

NULL      = /dev/null
BLACKHOLE = >$(NULL) 2>&1

CLEAN_CMD    = $(DC) $(CLEANMASK) $(BLACKHOLE)

!endif

.SUFFIXES:
.SUFFIXES:  .sym .exe .dll .lib .$(O) .res .inf .c .cpp .asm .h .hpp .inc .rc .pas .ipf

.rc.res: .AUTODEPEND
 $(RC) $(RCOPT) $<

.c.$(O): .AUTODEPEND
 $(SAY) Compiling $< $(LOG)
 $(CC) $(COPT) -fo=$^&.$(O) $< $(LOG)

.cpp.obj: .AUTODEPEND
 @echo Compiling $<
 @$(CPP) $(CPPOPT) -fo=$^&.$(O) $< $(LOG)

.asm.$(O): .AUTODEPEND
 $(SAY) Assembling $< $(LOG)
 $(ASM) $(ASMOPT) -fo=$^&.$(O) $< $(LOG)

.pas.exe: .AUTODEPEND
 @echo Compiling $<
 @$(PC) $(PCOPT) $(PROJ)

.asm.obj: .AUTODEPEND
 @echo Assembling $<
 @$(ASM) $(ASMOPT) $<

.ipf.inf: .symbolic
 $(SAY) Compiling $<
 @$(HC) -i $< $@

.exe: $(DIR)
.exe.sym: .symbolic
 $(SAY) Creating symbol file
 @$(AWK) -f $(ROOT)$(SEP)tools$(SEP)bin$(SEP)mapsym.awk <$^&.wmp >$^&.map
 @$(MAPSYM) $^&.map
 #-@del $^&.map $^&.wmp $^@
 #-@move $^&.sym $^@

.dll: $(DIR)
.dll.sym: .symbolic
 @$(AWK) -f $(ROOT)$(SEP)tools$(SEP)bin$(SEP)mapsym.awk <$^&.wmp >$^&.map
 @$(MAPSYM) $^&.map
 #-@del $^&.map $^&.wmp $^@
 #-@move $^&.sym $^@

#
# "$(MAKE) subdirs" enters each dir in $(DIRS)
# and does $(MAKE) $(TARGET) in each dir:
#
subdirs: .SYMBOLIC
 @for %%i in ($(DIRS)) do cd %%i && $(MAKE) $(TARGET) && cd ..
