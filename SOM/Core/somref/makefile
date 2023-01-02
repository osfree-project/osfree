# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = somref
TRGT = $(PROJ).dll
DLL = 1
DESC = SOMREF
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata
srcfiles = $(p)somref$(e)

#INSTALL_ADD = 1
!if 0
EXPORTS = 
!endif

!ifdef HOSTBLD
SRC = $(MYDIR)..$(SEP)..$(SEP)
OS2TK = $(%OS2TK)
!else
SRC = $(MYDIR)..$(SEP)
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45
!endif

SOM_DIR=$(SRC)contrib$(SEP)
SOM_INCL= -i=$(SOM_DIR)include &
		-i=$(SOM_DIR)somkpub$(SEP)include &
		-i=$(SOM_DIR)somtk$(SEP)include &
		-i=$(OS2TK)$(SEP)som$(SEP)include

!ifdef HOSTBLD
DEST    = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)common$(SEP)dll

!ifeq %OS OS2
ADD_OPTS = -D_PLATFORM_OS2_ -DSTRICT
ADD_LINKOPT = lib clib3r
defs = $(%WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
ADD_OPTS = -DSTRICT -bc -bm -br -dWIN32_LEAN_AND_MEAN
ADD_LINKOPT = lib clbrdll
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS WIN64
ADD_LINKOPT = lib clbrdll
ADD_OPTS = -D_WIN32 -DSTRICT -DBUILD_RHBSEH2 -bc -bm -br -dWIN32_LEAN_AND_MEAN
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS DOS
ADD_OPTS = 
defs = $(%WATCOM)$(SEP)h$(SEP)dos
!endif
!ifeq %OS LINUX
ADD_OPTS = -D_PLATFORM_UNIX_ -D_PLATFORM_X11_ -DSTRICT &
    -DHAVE_UNISTD_H -DHAVE_SIGNAL_H
ADD_LINKOPT = lib clib3r
defs = $(%WATCOM)$(SEP)lh
!endif

!else
DEST    = os2tk45$(SEP)som$(SEP)common$(SEP)dll

ADD_OPTS = -D_PLATFORM_OS2_ -DSTRICT
ADD_LINKOPT = lib clib3r
defs = $(%WATCOM)$(SEP)h$(SEP)os2

!endif

ADD_COPT = -dNDEBUG -i=$(defs) -dHAVE_CONFIG_H &
           $(SOM_INCL)  -i=$(BLD)host$(SEP)$(%HOST)$(SEP)$(RELDIR) $(ADD_OPTS)

#STACKSIZE = 0x20000
#UNI2H = 1
C = r

!ifdef HOSTBLD
!include $(%ROOT)tools/mk/tools.mk
!else
!include $(%ROOT)tools/mk/appsos2.mk
#!include $(%ROOT)tools/mk/libsos2.mk
!endif

.c: $(SRC)contrib$(SEP)somref$(SEP)src

.h: $(SRC)contrib$(SEP)include
