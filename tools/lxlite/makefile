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
#TRGT  = $(PROJ2).exe
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
TARGETS  = $(PATH)$(PROJ2).exe

!include $(%ROOT)/mk/toolspas.mk

$(PATH)lxlite.res: $(MYDIR)lxlite.rc

$(PATH)os2api.res: $(MYDIR)os2api.rc

$(PATH)$(PROJ).exe: $(PATH)$(PROJ1).exe $(MYDIR)os2api.rc
 @$(SAY) RESCMP   $^. $(LOG)
 wrc -bt=os2 -i=$(%WATCOM)$(SEP)h$(SEP)os2 $(defs) $(MYDIR)os2api.rc $(PATH)$(PROJ1).exe -fe=$@ -fo=$(PATH)os2api.res

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ2).exe $(MYDIR)lxlite.rc
 @$(SAY) RESCMP   $^. $(LOG)
 wrc -bt=os2 -i=$(%WATCOM)$(SEP)h$(SEP)os2 $(defs) $(MYDIR)lxlite.rc $(PATH)$(PROJ2).exe -fe=$@ -fo=$(PATH)$(PROJ).res

$(PATH)$(PROJ2).exe: $(MYDIR)$(PROJ).pas .symbolic
 @$(SAY) PPC      $^. $(LOG)
 $(verbose)$(PC) $(PCOPT) -o$^. -FE$^: -Fe$^: $<
