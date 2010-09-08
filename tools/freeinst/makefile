#
# A Makefile for freeinst
# (c) osFree project,
# valerius, 2010/07/14
#
# $Id: makefile,v 1.1 2004/08/16 06:27:30 prokushev Exp $
#

PROJ  = freeinst
!ifeq ENV OS/2
defs = -dOS2
!endif
!ifeq ENV WIN32
defs = -dWIN32
!endif
!ifeq ENV DOS
defs = -dDPMI32
!endif
!ifeq ENV LINUX
defs = -dLINUX
!endif
ADD_PCOPT = $(defs)
DESC  = FreeLDR installer by Yoda
!include $(%ROOT)/mk/toolspas.mk
