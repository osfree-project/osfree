# Makefile for SOM Compiler for

PROJ = somipc2
TRGT = $(PROJ).exe
DESC = SOM IPC
srcfiles = $(p)rhbsc$(e) &
	 $(p)rhbseh2$(e) &
	 $(p)rhbidl$(e) &
	 $(p)rhbscemt$(e) &
	 $(p)rhbsctyp$(e) &
	 $(p)rhbscpp$(e) &
	 $(p)rhbsctxt$(e) &
	 $(p)rhbscsome$(e)

HOSTBLD=1
PLATFORM = host$(SEP)$(%HOST)$(SEP)
!ifdef HOSTBLD
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)bin
OS2TK = $(%OS2TK)
SRC = $(MYDIR)
!ifeq %OS OS2
OPTS = -DHAVE_CONFIG_HPP
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45
defs = $(%WATCOM)$(SEP)h$(SEP)os2
ADD_LINKOPT = lib clib3r
!endif
!ifeq %OS WIN32
OPTS = -bm -br -dWIN32_LEAN_AND_MEAN -dHAVE_CONFIG_HPP
defs = $(%WATCOM)$(SEP)h$(SEP)nt
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS WIN64
OPTS = -bm -br -DBUILD_RHBSEH2 /D_USE_SOME_ /DWIN32_LEAN_AND_MEAN /D_WIN32 /DHAVE_CONFIG_HPP
defs = $(%WATCOM)$(SEP)h$(SEP)nt
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS DOS
OPTS =
defs = $(%WATCOM)$(SEP)h$(SEP)dos
ADD_LINKOPT = lib clib3r
!endif
!ifeq %OS LINUX
OPTS = -d_PLATFORM_UNIX_ -dHAVE_CONFIG_HPP
defs = $(%WATCOM)$(SEP)lh
ADD_LINKOPT = lib clib3r
!endif

!else
DEST = os2tk45$(SEP)som$(SEP)bin

SRC = $(MYDIR)
OPTS = -DHAVE_CONFIG_HPP
defs = $(%WATCOM)$(SEP)h$(SEP)os2
ADD_LINKOPT = lib clib3r

!endif

ADD_COPT = -dNDEBUG -i=$(defs) -bc $(OPTS) &
           -i=$(SRC) -i=$(OS2TK)$(SEP)som$(SEP)include -D_USE_SOME_

UNI2H = 1
C = r

!ifdef HOSTBLD
!include $(%ROOT)/mk/tools.mk
!else
!include $(%ROOT)/mk/appsos2.mk
!endif

.c: $(SRC)$(SEP)..$(SEP)somipc2