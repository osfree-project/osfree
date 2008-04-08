#
# A Makefile for qemu-img
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

DIR = ..$(SEP)bin
32_BITS  = 1
ADD_COPT = -dQEMU_TOOL -d__WATCOM__ -d__OS2__ -i=. -i=..$(SEP)include

OBJS = qemu-img.$(O) block.$(O) block-cow.$(O) block-qcow.$(O) aes.$(O) &
       block-vmdk.$(O) block-cloop.$(O) block-dmg.$(O) block-bochs.$(O) &
       block-vpc.$(O) block-vvfat.$(O) porting.$(O)

!include $(%ROOT)/mk/all.mk

CLEANMASK = *.lnk *.wmp *.obj *.err *.log *.bak *.sym

all: qemu-img.exe .SYMBOLIC

qemu-img.exe: $(OBJS)
 @%create $^&.lnk
# @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk ALIAS mmap_=_mmap
 @%append $^&.lnk ALIAS munmap_=_munmap
 @%append $^&.lnk ALIAS inflateInit2_=inflateInit2__
 @%append $^&.lnk ALIAS inflate=inflate_
 @%append $^&.lnk ALIAS inflateEnd=inflateEnd_
 @%append $^&.lnk ALIAS deflateInit2_=deflateInit2__
 @%append $^&.lnk ALIAS deflate=deflate_
 @%append $^&.lnk ALIAS deflateEnd=deflateEnd_
 @%append $^&.lnk ALIAS inflateInit_=inflateInit__
 @%append $^&.lnk ALIAS inflateReset=inflateReset_
 @%append $^&.lnk LIBRARY libz
 @%append $^&.lnk LIBRARY libmmap
 @%append $^&.lnk LIBPATH ..$(SEP)lib
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 @$(MAKE) $(MAKEOPT) install

install: .SYMBOLIC
 $(CP) qemu-img.exe $(DIR) $(BLACKHOLE)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
