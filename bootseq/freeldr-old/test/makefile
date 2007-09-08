#
# test
#

DEFINES    = -dOS2

!include ../../build.conf
!include ../../mk/site.mk

DIR = freeldr/test
ADD_COPT   = -od -s $(DEFINES) -i=.
ADD_ASMOPT =

STARTUP   =
ALL       = testc.$(O) testasm.$(O) vsprintf.$(O) video.$(O)

OBJS      = $(STARTUP) $(ALL)

!include ../../mk/bootseq.mk

all: test.exe

test.exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM dos
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk ALIAS _loadhigh=loadhigh
 @%append $^&.lnk ALIAS printk=printk_
 @%append $^&.lnk ALIAS _current_seg=current_seg
 @%append $^&.lnk LIBRARY $(LIBC)
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

