# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

DESCRIPTION = dos2unix
SOURCES = dos2unix querycp common &
!ifeq %OS WIN32
   wildargv
!endif


UCS = 1
DOS2UNIX_VERSION = 7.5.7
DOS2UNIX_VERSION_SHORT = 757
DOS2UNIX_DATE = 2026-08-27
DOS2UNIX_AUTHOR = 'Erwin Waterlander'

ADD_COPT = -w4 -e25 -zq -od -d2 -5r -mf

!ifeq %OS OS2
defs = -i=$(%WATCOM)$(SEP)h$(SEP)os2
ADD_COPT = $(ADD_COPT) -bt=os2
!endif
!ifeq %OS WIN32
defs = -i=$(%WATCOM)$(SEP)h$(SEP)nt
ADD_COPT = $(ADD_COPT) -bt=nt
UNIFILE = 1
!endif
!ifeq %OS DOS
defs = -i=$(%WATCOM)$(SEP)h$(SEP)dos
ADD_COPT = $(ADD_COPT) -bt=dos
!endif
!ifeq %OS LINUX
defs = -i=$(%WATCOM)$(SEP)lh
ADD_COPT = $(ADD_COPT) -bt=linux
!endif
DEFINES = -dVER_REVISION="$(DOS2UNIX_VERSION)" -dVER_DATE="$(DOS2UNIX_DATE)" -dVER_AUTHOR="$(DOS2UNIX_AUTHOR)" $(defs)
!ifeq UCS 1
ADD_COPT += -dD2U_UNICODE
!endif

!include $(%ROOT)tools/mk/build.mk
!include $(MYDIR)..$(SEP)dos2unix.mk


WATCOMSRC = $(%WATCOM)$(SEP)src$(SEP)startup

!ifeq UNIFILE 1
ADD_COPT += -dD2U_UNIFILE -d_O_U16TEXT=0x20000
!endif

ADD_LINKOPT = lib mpr
STACKSIZE = 0x20000
C = r

.c: $(PORT_BASE)dos2unix
.c: $(WATCOMSRC)

.h: $(PORT_BASE)dos2unix

.$(O): $(PATH)
