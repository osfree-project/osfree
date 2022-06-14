# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

TARGETS=$(ROOT)build$(SEP)include$(SEP)somref.h

ADD_SCOPT = -D__PRIVATE__ &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somref &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somd &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som


!include $(%ROOT)/mk/all.mk

.idl: $(MYDIR)..$(SEP)contrib$(SEP)somref
.h: $(ROOT)build$(SEP)include
