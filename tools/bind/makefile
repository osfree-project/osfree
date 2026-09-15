#
# A Makefile for BIND
# (c) osFree project,
# author, date
#

DESCRIPTION = Message binder
#defines object file names in format $(p)objname$(e)
SOURCES = bind 
# defines additional options for C compiler
#ADD_COPT = -sg #-I$(%OS2TK)$(SEP)h #-p


!ifndef TARGETBLD
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)bin
SRC  = $(MYDIR)
!else
ADD_COPT = -bt=os2
DEST = os2tk45$(SEP)bin
SRC  = $(MYDIR)..$(SEP)..$(SEP)bind$(SEP)
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

!include $(%ROOT)tools/mk/build.mk
