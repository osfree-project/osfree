# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

IDLH=$(ROOT)build$(SEP)include$(SEP)

TARGETS=$(IDLH)snglicls.h &
$(IDLH)sombacls.h &
$(IDLH)somida.h &
$(IDLH)somproxy.h &
$(IDLH)somsid.h &

ADD_SCOPT = -I$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som &
             -I$(MYDIR)..$(SEP)contrib$(SEP)somu

!include $(%ROOT)/mk/all.mk

.idl: $(MYDIR)..$(SEP)contrib$(SEP)somu
.h: $(IDLH)
