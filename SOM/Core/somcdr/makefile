# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $


PROJ = somcdr
TRGT = $(PROJ).dll
DLL = 1
DESC = SOM Kernel
srcfiles = $(p)rhbsomue$(e) &
	   $(p)somcdr$(e)

#$(p)rhbsomut$(e) & $(p)rhbsomkd$(e) &

ADD_SCOPT = -D__PRIVATE__ &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somd &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somestrm &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somnmf &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somabs1 &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somu &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somcorba &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somref &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somos &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somcslib &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somir


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

ADD_COPT = -dNDEBUG $(defs) -i=$(MYDIR)..$(SEP)contrib$(SEP)include -dHAVE_CONFIG_H &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)som$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)somtk$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)somcdr$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)rhbmtut$(SEP)include &
           -i=$(ROOT)build$(SEP)include -i=$(%OS2TK)$(SEP)som$(SEP)include

#STACKSIZE = 0x20000
C = r

!include $(%ROOT)/mk/tools.mk

IDLH=$(ROOT)build$(SEP)include$(SEP)

TARGETS= &
$(IDLH)somcdr.h &
$(IDLH)rhbsomuk.h &
$(TRGT)

$(p)rhbsomue$(e): $(IDLH)somcdr.h .SYMBOLIC

.idl: $(MYDIR)..$(SEP)contrib$(SEP)somcdr

.c: $(MYDIR)..$(SEP)contrib$(SEP)somcdr$(SEP)src

.h: $(IDLH);$(MYDIR)..$(SEP)contrib$(SEP)include
