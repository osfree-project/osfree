#
#       Makefile for OS/3 Loader. (FreeLdr) 06/09/29 valerius
#

#
#       Dedicated to Jesus Christ, my Lord and savior!
#
#       Copyright (C) 2005 by Sascha Schmidt
#
#       This makefile builds os2ldr, the osFree kernel loader
#

DEFINES    = -dOS2

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

DIR = freeldr
ADD_COPT   = -od -s $(DEFINES) -i=. -i=$(%ROOT)$(SEP)include
ADD_ASMOPT =

STARTUP    =
ALL        = freeldra.$(O) load.$(O) periphs.$(O) io.$(O) &
             video.$(O) vsprintf.$(O) filesys.$(O) util.$(O)

OBJS       = $(STARTUP) $(ALL)

!include $(%ROOT)/mk/bootseq.mk

all: os2ldr

os2ldr: os2ldr.com
 $(SAY) Making $^@ ... $(LOG)
 @$(RN) $< $^@ $(LOG)
 #$(MKBIN) $< $^@ $(LOG)

os2ldr.com: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM dos com
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk OPTION NODEFAULTLIBS
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk ALIAS _DispNTS=DispNTS
 @%append $^&.lnk ALIAS _DebugNTS=DebugNTS
 @%append $^&.lnk ALIAS _DispC=DispC
 @%append $^&.lnk ALIAS printk=_printk
 @%append $^&.lnk ALIAS _loadhigh=loadhigh
 @%append $^&.lnk ALIAS auxil=_auxil
 @%append $^&.lnk ALIAS Stage2Loader=Stage2Loader_
 @%append $^&.lnk ALIAS _BPBTable=BPBTable
 @%append $^&.lnk ALIAS _muOpen=muOpen
 @%append $^&.lnk ALIAS _muClose=muClose
 @%append $^&.lnk ALIAS _muRead=muRead
 @%append $^&.lnk ALIAS _muTerminate=muTerminate
 @%append $^&.lnk ALIAS _Int12Value=Int12Value
 @%append $^&.lnk ALIAS _Int1588Value=Int1588Value
 @%append $^&.lnk ALIAS _current_seg=current_seg
 @%append $^&.lnk LIBRARY $(LIBC)
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

#cstart_t.obj: startup.obj
# $(SAY) Making $^@ ... $(LOG)
# $(CP)  $< $^@ $(LOG)

#$(LIBC): $(LIBDIR)\$(LIBC)
# $(SAY) Making $(LIBC) ... $(LOG)
# $(RN) $< $^@ $(LOG)
# $(LIB) $^@ -*cstart.obj $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

