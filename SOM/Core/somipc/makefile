# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = somipc
TRGT = $(PROJ).exe
DESC = SOM IPC
srcfiles = $(p)rhbsc$(e) &
	 $(p)rhbsckih$(e) &
	 $(p)rhbidl$(e) &
	 $(p)rhbscemt$(e) &
	 $(p)rhbscir$(e) &
	 $(p)rhbsctyp$(e) &
	 $(p)rhbscpp$(e) &
	 $(p)rhbschdr$(e) &
	 $(p)rhbsctc$(e) &
	 $(p)rhbscapi$(e)


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

ADD_COPT = -dNDEBUG -i=$(defs) -i=$(MYDIR)..$(SEP)contrib$(SEP)include -i=$(MYDIR)..$(SEP)contrib$(SEP)somipc$(SEP)include -bc -bm -br -dWIN32_LEAN_AND_MEAN -dHAVE_CONFIG_HPP &
           -i=$(MYDIR)..$(SEP)contrib$(SEP)somirfmt$(SEP)include

ADD_LINKOPT = lib clbrdll
#STACKSIZE = 0x20000
#UNI2H = 1
C = r

!include $(%ROOT)/mk/tools.mk

.cpp: $(MYDIR)..$(SEP)contrib$(SEP)somipc$(SEP)src

.h: $(MYDIR)..$(SEP)contrib$(SEP)include;$(MYDIR)..$(SEP)contrib$(SEP)somipc$(SEP)include;$(MYDIR)..$(SEP)contrib$(SEP)somirfmt$(SEP)include

#.$(O): $(PATH)

