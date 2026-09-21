# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#

TARGET_CLASS = LIBRARY

!ifndef TARGETBLD
SRCDIR     = $(MYDIR)
!else
SRCDIR = $(%ROOT)tools$(SEP)shared$(SEP)omf$(SEP)
TARGET_API      = OS2
TARGET_SUBCLASS = STATIC
!endif

ADD_COPT  = -i=$(SRCDIR)..$(SEP)..$(SEP)include

!include $(%ROOT)tools/mk/build.mk

.c: $(SRCDIR)
