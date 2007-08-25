#
# Makefile for OS/3 loader uFSD's common code.
# 06/09/29 valerius
#

32_BITS  = 1

!include ../../../build.conf
!include ../../../mk/site.mk

ADD_ASMOPT =
DEFINES    = -dMICROFSD_TEST -dOS2 -d__KERNEL__ -d__WATCOM__

!ifeq UNIX FALSE

DIR        = bootseq\uFSD\lilo
MYDIR      = $(ROOT)\$(DIR)
LIBPATH    = $(MYDIR)\..
ADD_COPT   = $(DEFINES) -i=$(ROOT)\include\uFSD -i=.

!else

DIR        = bootseq/uFSD/lilo
MYDIR      = $(ROOT)/$(DIR)
LIBPATH    = $(MYDIR)/..
ADD_COPT   = $(DEFINES) -i=$(ROOT)/include/uFSD -i=.

!endif

DIRS       =

OBJS       = build.$(O) boot.$(O)

!include ../../../mk/bootseq.mk

all: boot.com build.exe .SYMBOLIC
 $(MAKE) TARGET=$^@ subdirs


build.exe: build.$(O)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk PATH .
 @%append $^&.lnk FILE $<
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)
 $(CP) $^@ $(TOOLS)

boot.com: boot.$(O)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM dos com
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk OPTION NODEFAULTLIBS
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk PATH .
 @%append $^&.lnk FILE $<
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

build.$(O): build.c
 wcc386 $< -fo=$^@ $(LOG)

boot.$(O): boot.asm
 wasm   $< -fo=$^@ $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) TARGET=$^@ subdirs
