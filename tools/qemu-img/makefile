#
# A Makefile for qemu-img
# (c) osFree project,
# valerius, 2006/10/30
#

!include ../../build.conf
!include ../../mk/site.mk

32_BITS  = 1
ADD_COPT = -dQEMU_TOOL -d__WATCOM__ -d__OS2__ -i=. -i=..\include

OBJS = qemu-img.$(O) block.$(O) block-cow.$(O) block-qcow.$(O) aes.$(O) &
       block-vmdk.$(O) block-cloop.$(O) block-dmg.$(O) block-bochs.$(O) &
       block-vpc.$(O) block-vvfat.$(O) porting.$(O)

!include ../../mk/bootseq.mk

CLEANMASK = *.lnk *.wmp *.obj *.err *.log *.bak *.sym

all: qemu-img.exe .SYMBOLIC

qemu-img.exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk ALIAS mmap_=_mmap
 @%append $^&.lnk ALIAS munmap_=_munmap
 @%append $^&.lnk LIBRARY libz
 @%append $^&.lnk LIBRARY libmmap
 @%append $^&.lnk LIBPATH ../lib
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

!ifeq UNIX FALSE

install: .SYMBOLIC
 $(CP) qemu-img.exe ..\bin

!else

install: .SYMBOLIC
 $(CP) qemu-img.exe ../bin

!endif

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
