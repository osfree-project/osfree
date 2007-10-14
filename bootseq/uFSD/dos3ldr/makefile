#
# Makefile for OS/3 DOS loader.
# 06/10/03 valerius
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

!ifeq UNIX FALSE

ADD_COPT   = -i=. -i=$(%ROOT)\include
ADD_ASMOPT = -i=. -i=$(%ROOT)\include
DIR        = bootseq\uFSD\dos3ldr
MYDIR      = $(ROOT)\$(DIR)

!else

ADD_COPT   = -i=. -i=$(%ROOT)/include
ADD_ASMOPT = -i=. -i=$(%ROOT)/include
DIR        = bootseq/uFSD/dos3ldr
MYDIR      = $(ROOT)/$(DIR)

!endif

OBJS       = dos3ldr.$(O) readhd.$(O) # diskread.$(O)

!include $(%ROOT)/mk/bootseq.mk

all: dos3ldr.exe

dos3ldr.exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM dos
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk NAME $^&
 @for %i in ($(OBJS)) do %append $^&.lnk FILE %i
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
