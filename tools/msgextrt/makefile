
DESCRIPTION = Extract Messages
SOURCES = msgextrt

!ifndef TARGETBLD
SRC  = $(MYDIR)
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)bin
!else
ADD_COPT = -bt=os2
DEST = os2tk45$(SEP)bin
SRC  = $(MYDIR)..$(SEP)..$(SEP)msgextrt$(SEP)
!endif

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

!ifdef TARGETBLD
TARGET_API = OS2
!endif

LIBS=tools_shared

!include $(%ROOT)tools/mk/build.mk

ADD_COPT += -dNDEBUG  -i=$(SRC) -i=$(defs) -i=$(%WATCOM)$(SEP)h$(SEP)os2 -za99
#ADD_LINKOPT = lib clib3r
STACKSIZE = 0x20000
C = r
