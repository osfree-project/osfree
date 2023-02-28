#
# A Makefile for freeinst
# (c) osFree project,
# valerius, 2010/07/14
#
# $Id: makefile,v 1.1 2004/08/16 06:27:30 prokushev Exp $
#

PROJ  = lxlite
# USE_RESOURCES = 1
!ifdef USE_RESOURCES
PROJ1 = lxlite1
!endif
#TRGT  = $(PROJ).exe
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
DESC = lxlite and utilities

!include $(%ROOT)tools/mk/toolspas.mk
!include $(MYDIR)lxlite.mk

ADD_PCOPT = $(defs) -Fi$(PORT_BASE)src -Fu$(PORT_BASE)src -Fu$(PORT_BASE)src$(SEP)common -duse32 -Rintel -So -Xm
TARGETS  = $(PATH)$(PROJ).exe

.pas: $(PORT_BASE)src$(SEP)common

.pas: $(PORT_BASE)src

.inc: $(PORT_BASE)src

!ifdef USE_RESOURCES
$(PATH)rc.rc: $(PORT_BASE)src$(SEP)os2api.rc $(PORT_BASE)src$(SEP)lxlite.rc
!ifeq UNIX TRUE
 $(verbose)cat $(PORT_BASE)src$(SEP)lxlite.rc $(PORT_BASE)src$(SEP)os2api.rc >$(PATH)rc.rc
!else
 $(verbose)copy /b $(PORT_BASE)src$(SEP)os2api.rc+$(PORT_BASE)src$(SEP)lxlite.rc $(PATH)rc.rc
!endif

$(PATH)$(PROJ).exe: $(PATH)$(PROJ1).exe $(PATH)rc.rc
 @$(SAY) RESCMP   $^. $(LOG)
 $(verbose)$(RC)  -zm $(defs) -i=$(%WATCOM)$(SEP)h$(SEP)os2 $(PATH)rc.rc $(PATH)$(PROJ1).exe -fe=$@ -fo=$(PATH)rc.res

$(PATH)$(PROJ1).exe: $(MYDIR)$(PROJ).pas .symbolic
 @$(SAY) PPC      $^. $(LOG)
 $(verbose)$(PC) $(PCOPT) -FE$(PATH) -Fe$(PATH) -o$(PROJ1).exe $(PROJ).pas
!endif
