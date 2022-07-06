#
# A Makefile for freeinst
# (c) osFree project,
# valerius, 2010/07/14
#
# $Id: makefile,v 1.1 2004/08/16 06:27:30 prokushev Exp $
#

PROJ  = lxlite
PROJ1 = lxlite1
PROJ2 = lxlite2
TRGT = $(PROJ2).exe
!ifeq %OS OS2
defs = -dOS2
!endif
!ifeq %OS WIN32
defs = -dWIN32
!endif
!ifeq %OS DOS
defs = -dDPMI32
!endif
!ifeq %OS LINUX
defs = -dLINUX
!endif
defs = $(defs) -dUSE32
DESC  = lxlite and utilities
ADD_PCOPT = $(defs) -Fucommon -duse32 -Rintel -So -Xm
#RESOURCE = $(PATH)$(PROJ).res

!include $(%ROOT)/mk/toolspas.mk

$(PATH)$(PROJ2).exe: $(PATH)$(PROJ1).exe $(MYDIR)os2api.rc
 @$(SAY) RESCMP   $^. $(LOG)
 $(RC) -bt=os2 -i=$(%WATCOM)$(SEP)h$(SEP)os2 -i=$(inc) $]@ $[@ -fe=$@ -fo=$(PATH)os2api.res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)$(PROJ).rc
 @$(SAY) RESCMP   $^. $(LOG)
 $(RC) -bt=os2 -i=$(%WATCOM)$(SEP)h$(SEP)os2 -i=$(inc) $]@ $[@ -fe=$@ -fo=$(PATH)$(PROJ).res
