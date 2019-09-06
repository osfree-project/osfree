#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

32_BITS  = 1
PROJ = genext2fs
TRGT = $(PROJ).exe
DESC = Generate ext2fs filesystem image
!ifeq %OS OS/2
defs = -d__OS2__
!endif
!ifeq %OS WIN32
defs = -d__NT__
!endif
!ifeq %OS DOS
defs = -dDPMI32
!endif
!ifeq %OS LINUX
defs = -dLINUX
!endif
DEFINES  = -hw -d3 -d__WATCOM__ $(defs)
ADD_COPT      = $(DEFINES) -i=.

srcfiles = $(p)getline$(e) $(p)strtof$(e) $(p)genext2fs$(e)

!include $(%ROOT)/mk/tools.mk
