#
# A Makefile for freeinst
# (c) osFree project,
# valerius, 2010/07/14
#
# $Id: makefile,v 1.1 2004/08/16 06:27:30 prokushev Exp $
#

PROJ = lxlite
TRGT = $(PROJ).exe
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
ADD_PCOPT = $(defs) -Fucommon -duse32 -Rintel -So
!include $(%ROOT)/mk/toolspas.mk
