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

!include ../../build.conf
!include ../../mk/site.mk

DIR = freeldr
ADD_COPT   = -od -s $(DEFINES) -i=.
ADD_ASMOPT =

STARTUP    =
ALL        = freeldra.$(O) start32.$(O) cmodel.$(O) segread.$(O) &
             strcpy.$(O) freeldrc.$(O) periphs.$(O) io.$(O) i4m.$(O) &
             video.$(O) vsprintf.$(O) common.$(O) biosinfo.$(O) &
             apm.$(O) vbe.$(O) bios.$(O)

OBJS       = $(STARTUP) $(ALL)

!include ../../mk/bootseq.mk

all: os2ldr

os2ldr: os2ldr.com
 $(SAY) Making $^@ ... $(LOG)
 @$(DC) $^@ $(LOG)
 @$(RN) $< $^@ $(LOG)
 #$(MKBIN) $< $^@ $(LOG)

os2ldr.com: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM dos com
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk OPTION NODEFAULTLIBS
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk ALIAS segread_=_segread
 @%append $^&.lnk ALIAS _DispNTS=DispNTS
 @%append $^&.lnk ALIAS _DebugNTS=DebugNTS
 @%append $^&.lnk ALIAS _DispC=DispC
 @%append $^&.lnk ALIAS printf=_printf
 @%append $^&.lnk ALIAS printk=printk_
 @%append $^&.lnk ALIAS auxil=_auxil
 @%append $^&.lnk ALIAS KernelLoader=_KernelLoader
 @%append $^&.lnk ALIAS _muOpen=muOpen
 @%append $^&.lnk ALIAS _muClose=muClose
 @%append $^&.lnk ALIAS _muRead=muRead
 @%append $^&.lnk ALIAS _muTerminate=muTerminate
 @%append $^&.lnk ALIAS _Int12Value=Int12Value
 @%append $^&.lnk ALIAS _Int1588Value=Int1588Value
 @%append $^&.lnk ALIAS _current_seg=current_seg
 @%append $^&.lnk ALIAS mbi=_mbi
 @%append $^&.lnk ALIAS entry_addr=_entry_addr
 @%append $^&.lnk ALIAS _loadhigh=loadhigh
 @%append $^&.lnk ALIAS _get_memsize=get_memsize
 @%append $^&.lnk ALIAS _get_eisamemsize=get_eisamemsize
 @%append $^&.lnk ALIAS _get_mmap_entry=get_mmap_entry
 @%append $^&.lnk ALIAS _get_rom_config_table=get_rom_config_table
 @%append $^&.lnk ALIAS _get_apm_info=get_apm_info
 @%append $^&.lnk ALIAS _get_code_end=get_code_end
 @%append $^&.lnk ALIAS _get_diskinfo_standard=get_diskinfo_standard
 @%append $^&.lnk ALIAS _check_int13_extensions=check_int13_extensions
 @%append $^&.lnk ALIAS _biosdisk_standard=biosdisk_standard
 @%append $^&.lnk ALIAS _biosdisk_int13_extensions=biosdisk_int13_extensions
 @%append $^&.lnk ALIAS apm_bios_info=_apm_bios_info
 @%append $^&.lnk ALIAS reset_vbe_mode_=reset_vbe_mode
 @%append $^&.lnk ALIAS get_vbe_controller_info_=get_vbe_controller_info
 @%append $^&.lnk ALIAS get_vbe_mode_info_=get_vbe_mode_info
 @%append $^&.lnk ALIAS set_vbe_mode_=set_vbe_mode
 @%append $^&.lnk ALIAS get_vbe_pmif_=get_vbe_pmif
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

