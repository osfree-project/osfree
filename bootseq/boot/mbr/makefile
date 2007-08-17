#
# Makefile for OS/3 loader MBR sector.
# 06/10/03 valerius
#

!include ../../../build.conf
!include ../../../mk/site.mk

DEFINES    =
ADD_ASMOPT = $(DEFINES)

!ifeq UNIX FALSE

DIR        = boot\mbr
MYDIR      = $(ROOT)\$(DIR)

!else

DIR        = boot/mbr
MYDIR      = $(ROOT)/$(DIR)

!endif

STARTUP    =
ALL        = mbr.$(O)
OBJS       = $(STARTUP) $(ALL)

!include ../../../mk/bootseq.mk

.SUFFIXES:
.SUFFIXES:  .bin .exe .$(O) .asm

all: mbr.bin

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

.asm.$(O):
 $(SAY) Assembling $< $(LOG)
 $(ASM) $(ASMOPT) -fo=$^@ $< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
