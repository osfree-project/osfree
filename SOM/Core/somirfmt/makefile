# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = somirfmt
TRGT = $(PROJ).lib
srcfiles = $(p)somirfmt$(e) $(p)somirutl$(e)

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

!ifdef HOSTBLD
SRC = $(MYDIR)..$(SEP)..$(SEP)
!else
SRC = $(MYDIR)..$(SEP)
!endif

ADD_COPT = -dNDEBUG -bc -bm -br -dWIN32_LEAN_AND_MEAN -dHAVE_CONFIG_H &
           -i=$(defs) -i=$(SRC)contrib$(SEP)include &
           -i=$(SRC)contrib$(SEP)somir$(SEP)include &
           -i=$(SRC)contrib$(SEP)somirfmt$(SEP)include &
           -i=$(SRC)contrib$(SEP)rhbmtut$(SEP)include &
           -i=$(SRC)contrib$(SEP)somtk$(SEP)include &
           -i=$(SRC)contrib$(SEP)somtc$(SEP)include &
           -i=$(SRC)contrib$(SEP)somkpub$(SEP)include &
           -i=$(ROOT)build$(SEP)include

ADD_LINKOPT = lib clbrdll
#STACKSIZE = 0x20000
#UNI2H = 1
C = r

TARGETS = $(PATH)$(TRGT)
SRCDIR = $(PATH)

!include $(%ROOT)/mk/libs.mk

.c: $(SRC)contrib$(SEP)somirfmt$(SEP)src

.h: $(SRC)contrib$(SEP)include;$(SRC)contrib$(SEP)somir$(SEP)include;$(SRC)contrib$(SEP)somkpub$(SEP)include

DEST      = $(BLD)$(SEP)lib

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
