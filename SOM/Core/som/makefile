# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = som
TRGT = $(PROJ).dll
DLL = 1
DESC = SOM Kernel
srcfiles = $(p)somkern$(e) &
	 $(p)rhbsomid$(e) &
	 $(p)somalloc$(e) &
	 $(p)somobjva$(e) &
	 $(p)somapi$(e) &
	 $(p)sommutex$(e) &
	 $(p)somkpath$(e) &
	 $(p)somshlb$(e)


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
           -i=$(MYDIR)..$(SEP)contrib$(SEP)somcdr$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)rhbmtut$(SEP)include &
           -i=$(ROOT)build$(SEP)include

ADD_LINKOPT = lib clbrdll
#STACKSIZE = 0x20000
#UNI2H = 1
C = r

!include $(%ROOT)/mk/tools.mk

.c: $(MYDIR)..$(SEP)contrib$(SEP)som$(SEP)src

.h: $(MYDIR)..$(SEP)contrib$(SEP)include

#.$(O): $(PATH)

