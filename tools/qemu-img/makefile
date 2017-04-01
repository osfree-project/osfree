#
# A Makefile for qemu-img
# (c) osFree project,
# valerius, 2006/10/30
#

PROJ = qemu-img
TRGT = $(PROJ).exe
DESC = Generate/convert HDD images
#TARGETS = $(PROJ).exe $(PROJ)
srcfiles = $(p)qemu-img$(e) $(p)block$(e) $(p)block-cow$(e) $(p)block-qcow$(e) $(p)aes$(e) &
           $(p)block-vmdk$(e) $(p)block-cloop$(e) $(p)block-dmg$(e) $(p)block-bochs$(e) &
           $(p)block-vpc$(e) $(p)block-vvfat$(e) $(p)porting$(e)

!ifeq ENV OS/2
defs = -d__OS2__
!endif
!ifeq ENV WIN32
defs = -d_WIN32
!endif
!ifeq ENV DOS
defs = -dDPMI32
!endif
!ifeq ENV LINUX
defs = -dLINUX
!endif

!include $(%ROOT)/mk/tools.mk

ADD_COPT += $(defs) -dQEMU_TOOL -i=$(MYDIR) -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT = $(ADD_LINKOPT) ALIAS mmap_=_mmap, munmap_=_munmap
