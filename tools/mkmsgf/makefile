# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = mkmsgf
TRGT = $(PROJ).exe
DESC = Make Message
srcfiles = $(p)mkmsgf$(e)
!ifeq %OS OS2
defs = $(%WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS DOS
defs = $(%WATCOM)$(SEP)h$(SEP)dos
!endif
!ifeq %OS LINUX
defs = $(%WATCOM)$(SEP)lh
!endif

!include $(%ROOT)tools/mk/tools.mk
!include $(MYDIR)mkmsgf.mk

ADD_COPT = -dNDEBUG -i=$(PORT_BASE)src$(SEP)H -i=$(defs) -i=$(%WATCOM)$(SEP)h$(SEP)os2 -za99
ADD_LINKOPT = lib clib3r
STACKSIZE = 0x20000
C = r

.c: $(PORT_BASE)src

.h: $(PORT_BASE)src$(SEP)H

