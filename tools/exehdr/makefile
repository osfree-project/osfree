# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = exehdr
TRGT = $(PROJ).exe
DESC = Exe Header Dump
srcfiles = $(p)exehdr$(e) $(p)lxdisplay$(e) $(p)lxutils$(e) $(p)dumprawlx$(e)

!ifndef TARGETBLD
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)bin
SRC  = $(MYDIR)
!else
ADD_COPT = -bt=os2
DEST = os2tk45$(SEP)bin
SRC  = $(MYDIR)..$(SEP)..$(SEP)exehdr$(SEP)
!endif

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

!ifndef TARGETBLD
!include $(%ROOT)tools/mk/tools.mk
!else
!include $(%ROOT)tools/mk/appsos2_cmd.mk
!endif
!include $(MYDIR)exehdr.mk

ADD_COPT += -dNDEBUG -i=$(PORT_BASE)src$(SEP)H -i=$(defs) -i=$(%WATCOM)$(SEP)h$(SEP)os2 -za99
ADD_LINKOPT = lib clib3r
STACKSIZE = 0x20000
C = r

.c: $(PORT_BASE)src

.h: $(PORT_BASE)src

