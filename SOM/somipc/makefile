
PROJ = somipc
TRGT = $(PROJ).exe
srcfiles = $(p)rhbsc$(e) &
	 $(p)rhbseh2$(e) &
	 $(p)rhbsckih$(e) &
	 $(p)rhbidl$(e) &
	 $(p)rhbscemt$(e) &
	 $(p)rhbscir$(e) &
	 $(p)rhbsctyp$(e) &
	 $(p)rhbscpp$(e) &
	 $(p)rhbschdr$(e) &
	 $(p)rhbsctc$(e) &
	 $(p)rhbscapi$(e)

DEST = os2tk45$(SEP)som$(SEP)bin

!include $(%ROOT)SOM/common.mk

ADD_COPT = $(ADD_COPT) -i=$(PORT_BASE)somipc$(SEP)include -i=$(PORT_BASE)somirfmt$(SEP)include

.cpp: $(PORT_BASE)somipc$(SEP)src

.c: $(PORT_BASE)rhbseh2$(SEP)src

.h: $(PORT_BASE)somipc$(SEP)include;$(PORT_BASE)somirfmt$(SEP)include

