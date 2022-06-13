# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = somirfmt
TRGT = $(PROJ).lib
srcfiles = $(p)somirfmt$(e)

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

ADD_COPT = -dNDEBUG -bc -bm -br -dWIN32_LEAN_AND_MEAN -dHAVE_CONFIG_H &
           -i=$(defs) -i=$(MYDIR)..$(SEP)contrib$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)include &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)include

ADD_LINKOPT = lib clib3r
#STACKSIZE = 0x20000
#UNI2H = 1
C = r

TARGETS = $(PATH)$(TRGT)
SRCDIR = $(PATH)

!include $(%ROOT)/mk/libs.mk

.c: $(MYDIR)..$(SEP)contrib$(SEP)somirfmt$(SEP)src

.h: $(MYDIR)..$(SEP)contrib$(SEP)include;$(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)include;$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)include

DEST      = $(BLD)$(SEP)lib

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
