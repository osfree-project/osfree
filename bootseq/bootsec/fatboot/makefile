#
# Makefile for OS/3 loader MBR sector.
# 06/10/03 valerius
#

#DEFINES    = -dOS2LDR
DEFINES    =
ADD_ASMOPT = $(DEFINES)

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

!ifeq UNIX FALSE

DIR        = bootseq\bootsec\fatboot
MYDIR      = $(ROOT)\$(DIR)

!else

DIR        = bootseq/bootsec/fatboot
MYDIR      = $(ROOT)/$(DIR)

!endif

STARTUP    =
ALL        = fatboot.$(O)
OBJS       = $(STARTUP) $(ALL)

!include $(%ROOT)/mk/bootseq.mk

.SUFFIXES:
.SUFFIXES:  .bin .exe .com .$(O) .asm

all: fatboot.bin makefile

.com.bin:
 $(DC) $^@
 #$(MKBIN) $< $^@
 $(CP) $< $^@
 $(DC) $<


.$(O).com:
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM dos com
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk OPTION NODEFAULTLIBS
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk PATH $(MYDIR)
 @%append $^&.lnk FILE $<
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

.asm.$(O):
 $(SAY) Assembling $< $(LOG)
 $(ASM) $(ASMOPT) -fo=$^@ $< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
