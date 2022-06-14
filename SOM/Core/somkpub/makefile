# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

TARGETS=$(ROOT)build$(SEP)include$(SEP)somobj.h &
        $(ROOT)build$(SEP)include$(SEP)somcm.h &
        $(ROOT)build$(SEP)include$(SEP)somcls.h

ADD_SCOPT = -D__PRIVATE__ &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somd

!include $(%ROOT)/mk/all.mk

.idl: $(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som
.h: $(ROOT)build$(SEP)include

$(ROOT)build$(SEP)include$(SEP)somobj.h: $(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som$(SEP)somobj.idl
$(ROOT)build$(SEP)include$(SEP)somcm.h: $(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som$(SEP)somcm.idl
$(ROOT)build$(SEP)include$(SEP)somcls.h: $(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som$(SEP)somcls.idl