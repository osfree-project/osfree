# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#

!ifeq %OS LINUX
defs = -d__LINUX__ -i=$(%WATCOM)$(SEP)lh
!else
defs = 
!endif

ADD_COPT  = -i=$(MYDIR)..$(SEP)..$(SEP)include $(defs)

TARGET_CLASS = LIBRARY

!include $(%ROOT)tools/mk/build.mk
