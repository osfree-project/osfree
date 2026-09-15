#
# A Makefile for qemu-img
# (c) osFree project,
# valerius, 2006/10/30
#

DESCRIPTION = Generate/convert HDD images
SOURCES  = qemu-img block block-qcow aes &
           block-vmdk block-cloop block-dmg block-bochs &
           block-vpc block-vvfat porting # block-cow
TARGET_API = HOST
TARGET_BITS = 32

!ifeq %OS OS2
defs = -d__OS2__
!endif
!ifeq %OS WIN32
defs = -d_WIN32
!endif
!ifeq %OS DOS
defs = -dDPMI32
!endif
!ifeq %OS LINUX
defs = -dLINUX
!endif

ADD_COPT += $(defs) -dQEMU_TOOL -i=$(MYDIR) -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT = lib z.lib

!include $(%ROOT)tools/mk/build.mk
