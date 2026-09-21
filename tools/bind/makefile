#
# A Makefile for BIND
# (c) osFree project
#

DESCRIPTION = Message binder
SOURCES = bind
LIBS = tools_shared

!ifndef TARGETBLD
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)bin
SRC  = $(MYDIR)
!else
TARGET_API = OS2
ADD_COPT = -bt=os2
DEST = os2tk45$(SEP)bin
SRC  = $(MYDIR)..$(SEP)..$(SEP)bind$(SEP)
!endif

ADD_COPT = -i=$(SRC)..$(SEP)include

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


!include $(%ROOT)tools/mk/build.mk
