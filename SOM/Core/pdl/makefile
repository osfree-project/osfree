# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = pdl
TRGT = $(PROJ).exe
DESC = pdl
srcfiles = $(p)pdl$(e)

!ifeq %OS OS2
defs = $(%WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS WIN64
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS DOS
defs = $(%WATCOM)$(SEP)h$(SEP)dos
!endif
!ifeq %OS LINUX
defs = $(%WATCOM)$(SEP)lh
!endif

ADD_COPT = -dNDEBUG -i=$(defs) -i=$(MYDIR)..$(SEP)contrib$(SEP)include -bc -bm -br -dWIN32_LEAN_AND_MEAN -dHAVE_CONFIG_H
ADD_LINKOPT = lib clib3r
#STACKSIZE = 0x20000
#UNI2H = 1
C = r

!include $(%ROOT)/mk/tools.mk

.c: $(MYDIR)..$(SEP)contrib$(SEP)pdl$(SEP)src

.h: $(MYDIR)..$(SEP)contrib$(SEP)include

#.$(O): $(PATH)

