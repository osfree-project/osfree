# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = pdl
TRGT = $(PROJ).exe
DESC = pdl
srcfiles = $(p)pdl$(e)

DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)bin

!ifeq %OS OS2
OPTS =
defs = $(%WATCOM)$(SEP)h$(SEP)os2
ADD_LINKOPT = lib clib3r
!endif
!ifeq %OS WIN32
OPTS = -br -dWIN32_LEAN_AND_MEAN
defs = $(%WATCOM)$(SEP)h$(SEP)nt
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS WIN64
OPTS = -br -dWIN32_LEAN_AND_MEAN
defs = $(%WATCOM)$(SEP)h$(SEP)nt
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS DOS
OPTS =
defs = $(%WATCOM)$(SEP)h$(SEP)dos
ADD_LINKOPT = lib clib3r
!endif
!ifeq %OS LINUX
OPTS =
defs = $(%WATCOM)$(SEP)lh
ADD_LINKOPT = lib clib3r
!endif

ADD_COPT = -dNDEBUG -i=$(defs) -i=$(MYDIR)..$(SEP)contrib$(SEP)include -bc -bm -dHAVE_CONFIG_H $(OPTS)
#STACKSIZE = 0x20000
#UNI2H = 1
C = r

!include $(%ROOT)/mk/tools.mk

.c: $(MYDIR)..$(SEP)contrib$(SEP)pdl$(SEP)src

.h: $(MYDIR)..$(SEP)contrib$(SEP)include

#.$(O): $(PATH)

