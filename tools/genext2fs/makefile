#
# A Makefile for genext2fs
# (c) osFree project,
# valerius, 2006/10/30
#

32_BITS  = 1
PROJ = genext2fs
TRGT = $(PROJ).exe
DESC = Generate ext2fs filesystem image
!ifeq ENV OS/2
defs = -d__OS2__
!endif
!ifeq ENV WIN32
defs = -d__NT__
!endif
!ifeq ENV DOS
defs = -dDPMI32
!endif
!ifeq ENV LINUX
defs = -d_GNU_SOURCE
!endif
DEFINES  = -hw -d3 -d__WATCOM__ $(defs)
ADD_COPT      = $(DEFINES) -i=.

srcfiles = $(p)getline$(e) $(p)strtof$(e) $(p)genext2fs$(e)

!include $(%ROOT)/mk/tools.mk
