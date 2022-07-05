#
# A Makefile for mkmsgf
# (c) osFree project,
# valerius, 2006/10/30
#
# $Id: makefile,v 1.1 2004/08/16 06:27:30 prokushev Exp $
#

PROJ = mkmsgf
TRGT = $(PROJ).exe
DESC = Make message file tool

!ifndef TARGETBLD
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)bin
!include $(%ROOT)/mk/toolspas.mk
!else
DEST = os2tk45$(SEP)bin
SRC = $(MYDIR)..$(SEP)..$(SEP)mkmsgf$(SEP)
!include $(%ROOT)/mk/appsos2pas.mk
!endif

.pas: $(SRC)
