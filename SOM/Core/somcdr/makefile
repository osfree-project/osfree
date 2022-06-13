# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

IDLH=$(ROOT)build$(SEP)include$(SEP)

PROJ = somcdr
TRGT = $(PROJ).dll
DLL = 1
DESC = SOM Kernel
srcfiles = $(p)rhbsomut$(e) &
	   $(p)rhbsomkd$(e) &
	   $(p)rhbsomue$(e) &
	   $(p)somcdr$(e)


TARGETS=$(TRGT) &
$(IDLH)somcdr.h &
$(IDLH)rhbsomuk.h &

ADD_SCOPT = -I$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som &
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
defs = $(%WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS WIN64
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS DOS
defs = $(%WATCOM)$(SEP)h$(SEP)dos
!endif
!ifeq %OS LINUX
defs = $(%WATCOM)$(SEP)lh
!endif

ADD_COPT = -dNDEBUG -i=$(defs) -i=$(MYDIR)..$(SEP)contrib$(SEP)include -bc -bm -br -dWIN32_LEAN_AND_MEAN -dHAVE_CONFIG_H &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)som$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)somtk$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)somcdr$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)rhbmtut$(SEP)include &
           -i=$(ROOT)build$(SEP)include

ADD_LINKOPT = lib clbrdll
#STACKSIZE = 0x20000
C = r

!include $(%ROOT)/mk/tools.mk

.idl: $(MYDIR)..$(SEP)contrib$(SEP)somcdr

.c: $(MYDIR)..$(SEP)contrib$(SEP)somcdr$(SEP)src

.h: $(IDLH);$(MYDIR)..$(SEP)contrib$(SEP)include
