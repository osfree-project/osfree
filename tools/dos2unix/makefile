# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = unix2dos
TRGT = $(PROJ).exe
DESC = unix2dos
srcfiles = $(p)unix2dos$(e) $(p)querycp$(e) $(p)common$(e) $(p)wildargv$(e)

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

!include $(%ROOT)tools/mk/tools.mk
!include $(MYDIR)dos2unix.mk


DOS2UNIX_VERSION = 7.5.7
DOS2UNIX_VERSION_SHORT = 757
DOS2UNIX_DATE = 2026-08-27
DOS2UNIX_AUTHOR = 'Erwin Waterlander'
UCS = 1
UNIFILE = 1
WATCOMSRC = $(%WATCOM)\src\startup
DEFINES = -DVER_REVISION="$(DOS2UNIX_VERSION)" -DVER_DATE="$(DOS2UNIX_DATE)" -DVER_AUTHOR="$(DOS2UNIX_AUTHOR)"
ADD_COPT = $(DEFINES) -w4 -e25 -zq -od -d2 -5r -bt=nt -mf
!ifeq UCS 1
ADD_COPT += -DD2U_UNICODE
!endif

!ifeq UNIFILE 1
ADD_COPT += -DD2U_UNIFILE -D_O_U16TEXT=0x20000
!endif

ADD_LINKOPT = lib mpr
STACKSIZE = 0x20000
C = r

.c: $(PORT_BASE)dos2unix
.c: $(WATCOMSRC)

.h: $(PORT_BASE)dos2unic

.$(O): $(PATH)
