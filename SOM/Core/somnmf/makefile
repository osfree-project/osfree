# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

IDLH=$(ROOT)build$(SEP)include$(SEP)

TARGETS=$(IDLH)biter.h &
lname.h &
lnamec.h &
naming.h &
xnaming.h &
xnamingf.h

ADD_SCOPT = -D__PRIVATE__ &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somref &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somnmf &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somd &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somu &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somir &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somcslib &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somabs1 &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somos

!include $(%ROOT)/mk/all.mk

.idl: $(MYDIR)..$(SEP)contrib$(SEP)somnmf
.h: $(IDLH)
