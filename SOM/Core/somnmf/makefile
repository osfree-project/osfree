# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

IDLH=$(ROOT)build$(SEP)include$(SEP)

TARGETS=$(IDLH)biter.h &
lname.h &
lnamec.h &
naming.h &
xnaming.h &
xnamingf.h

!ifdef HOSTBLD
SRC = $(MYDIR)..$(SEP)..$(SEP)
!else
SRC = $(MYDIR)..$(SEP)
!endif

ADD_SCOPT = -D__PRIVATE__ &
            -I$(SRC)contrib$(SEP)somkpub$(SEP)som &
            -I$(SRC)contrib$(SEP)somref &
            -I$(SRC)contrib$(SEP)somnmf &
            -I$(SRC)contrib$(SEP)somd &
            -I$(SRC)contrib$(SEP)somu &
            -I$(SRC)contrib$(SEP)somir &
            -I$(SRC)contrib$(SEP)somcslib &
            -I$(SRC)contrib$(SEP)somabs1 &
            -I$(SRC)contrib$(SEP)somos

!include $(%ROOT)tools/mk/all.mk

.idl: $(SRC)contrib$(SEP)somnmf
.h: $(IDLH)
