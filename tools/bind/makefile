#
# A Makefile for BIND
# (c) osFree project,
# author, date
#

PROJ = bind
TRGT = $(PROJ).exe
DESC = Binder
#defines object file names in format $(p)objname$(e)
srcfiles = $(p)bind$(e) 
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

!ifndef TARGETBLD
!include $(%ROOT)tools/mk/tools.mk
!else
!include $(%ROOT)tools/mk/appsos2_cmd.mk
!endif

