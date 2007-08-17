#
# Makefile for OS/3 loader bootsector.
# 06/10/03 valerius
#

#
# There are three variants of the bootsector.
# 1st variant (bootsect_1.bin) supports only
# contiguous muFSD's, e.g., in a partition
# boot block. The 2nd variant (bootsect.bin) is
# suitable for loading muFSD from a fragmented
# file from partition data area, like from FAT
# or foreign (like ext2fs etc.) filesystem
# without a special boot block. (but the size of
# file is limited to 64 Kb.) The 3rd variant is
# suitable for loading fragmented files of
# arbitrary size.
#

!include ../../../build.conf
!include ../../../mk/site.mk

DEFINES    =
DEFINES_1  = -dCODE_1 -dIBM_FAT_COMPAT
DEFINES_2  = -dCODE_2
DEFINES_3  = -dCODE_3
ADD_ASMOPT = $(DEFINES)

!ifeq UNIX FALSE

DIR        = bootseq\boot\bootsect
MYDIR      = $(ROOT)\$(DIR)

!else

DIR        = bootseq/boot/bootsect
MYDIR      = $(ROOT)/$(DIR)

!endif

STARTUP    =
ALL_1      = bootsect_1.$(O)
ALL_2      = bootsect_2.$(O)
ALL_3      = bootsect_3.$(O)

OBJS_1       = $(STARTUP) $(ALL_1)
OBJS_2       = $(STARTUP) $(ALL_2)
OBJS_3       = $(STARTUP) $(ALL_3)

!include ../../../mk/bootseq.mk

.SUFFIXES:
.SUFFIXES: .bin .exe .$(O) .asm

all: bootsect_1.bin bootsect_2.bin bootsect_3.bin

.exe.bin:
 $(DC) $^@
 $(MKBIN) $< $^@
 $(DC) $<

.$(O).exe:
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM dos
 @%append $^&.lnk OPTION MAP=$^&.map
 @%append $^&.lnk OPTION NODEFAULTLIBS
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk PATH $(MYDIR)
 @%append $^&.lnk FILE $<
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

bootsect_1.$(O): bootsect.asm
 $(SAY) Assembling $< $(LOG)
 $(ASM) $(ASMOPT) $(DEFINES_1) -fo=$^@ $< $(LOG)

bootsect_2.$(O): bootsect.asm
 $(SAY) Assembling $< $(LOG)
 $(ASM) $(ASMOPT) $(DEFINES_2) -fo=$^@ $< $(LOG)

bootsect_3.$(O): bootsect.asm
 $(SAY) Assembling $< $(LOG)
 $(ASM) $(ASMOPT) $(DEFINES_3) -fo=$^@ $< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
