# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

IDLH=$(ROOT)build$(SEP)include$(SEP)

TARGETS=$(IDLH)dynany.h

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

!ifdef HOSTBLD
SRC = $(MYDIR)..$(SEP)..$(SEP)
!else
SRC = $(MYDIR)..$(SEP)
!endif

!include $(%ROOT)tools/mk/all.mk

.idl: $(SRC)contrib$(SEP)somany
.h: $(IDLH)
