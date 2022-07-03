# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $


PROJ = somcdr
TRGT = $(PROJ).dll
DLL = 1
DESC = SOM Kernel
srcfiles = $(p)rhbsomue$(e) &
	   $(p)somcdr$(e)

#$(p)rhbsomut$(e) & $(p)rhbsomkd$(e) &

!ifdef HOSTBLD
SRC = $(MYDIR)..$(SEP)..$(SEP)
!else
SRC = $(MYDIR)..$(SEP)
!endif

ADD_SCOPT = -D__PRIVATE__ &
            -I$(SRC)contrib$(SEP)somkpub$(SEP)som &
            -I$(SRC)contrib$(SEP)somd &
            -I$(SRC)contrib$(SEP)somestrm &
            -I$(SRC)contrib$(SEP)somnmf &
            -I$(SRC)contrib$(SEP)somabs1 &
            -I$(SRC)contrib$(SEP)somu &
            -I$(SRC)contrib$(SEP)somcorba &
            -I$(SRC)contrib$(SEP)somref &
            -I$(SRC)contrib$(SEP)somos &
            -I$(SRC)contrib$(SEP)somcslib &
            -I$(SRC)contrib$(SEP)somir


!ifdef HOSTBLD

!ifeq %OS OS2
defs = -d_PLATFORM_OS2_ -i=$(%WATCOM)$(SEP)h$(SEP)os2
ADD_LINKOPT = lib clib3r
!endif
!ifeq %OS WIN32
defs = -bc -bm -br -dWIN32_LEAN_AND_MEAN -i=$(%WATCOM)$(SEP)h$(SEP)nt
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS WIN64
defs = -bc -bm -br -dWIN32_LEAN_AND_MEAN -i=$(%WATCOM)$(SEP)h$(SEP)nt
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS DOS
defs = -d_PLATFORM_DOS_ -i=$(%WATCOM)$(SEP)h$(SEP)dos
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS LINUX
defs = -d_PLATFORM_UNIX_ -d_PLATFORM_X11_ -dHAVE_SIGNAL_H -i=$(%WATCOM)$(SEP)lh
ADD_LINKOPT = lib clib3r
!endif

!else

defs = -d_PLATFORM_OS2_ -i=$(%WATCOM)$(SEP)h$(SEP)os2
ADD_LINKOPT = lib clib3r

!endif

ADD_COPT = -dNDEBUG $(defs) -i=$(SRC)contrib$(SEP)include -dHAVE_CONFIG_H &
           -i=$(SRC)contrib$(SEP)som$(SEP)include &
           -i=$(SRC)contrib$(SEP)somtk$(SEP)include &
           -i=$(SRC)contrib$(SEP)somcdr$(SEP)include &
           -i=$(SRC)contrib$(SEP)somkpub$(SEP)include &
           -i=$(SRC)contrib$(SEP)rhbmtut$(SEP)include &
           -i=$(ROOT)build$(SEP)include -i=$(%OS2TK)$(SEP)som$(SEP)include

#STACKSIZE = 0x20000
C = r

!ifdef HOSTBLD
!include $(%ROOT)/mk/tools.mk
!else
!include $(%ROOT)/mk/appsos2.mk
!endif

IDLH=$(ROOT)build$(SEP)include$(SEP)

TARGETS= &
$(IDLH)somcdr.h &
$(IDLH)rhbsomuk.h &
$(TRGT)

$(p)rhbsomue$(e): $(IDLH)somcdr.h .SYMBOLIC

.idl: $(SRC)contrib$(SEP)somcdr

.c: $(SRC)contrib$(SEP)somcdr$(SEP)src

.h: $(IDLH);$(SRC)contrib$(SEP)include
