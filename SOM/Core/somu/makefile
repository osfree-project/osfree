# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

IDLH=$(ROOT)build$(SEP)include$(SEP)

TARGETS=$(IDLH)snglicls.h &
$(IDLH)sombacls.h &
$(IDLH)somida.h &
$(IDLH)somproxy.h &
$(IDLH)somsid.h &

!ifdef HOSTBLD
SRC = $(MYDIR)..$(SEP)..$(SEP)
!else
SRC = $(MYDIR)..$(SEP)
!endif

ADD_SCOPT = -D__PRIVATE__ &
            -I$(SRC)contrib$(SEP)somkpub$(SEP)som &
            -I$(SRC)contrib$(SEP)somu

!include $(%ROOT)tools/mk/all.mk

.idl: $(SRC)contrib$(SEP)somu
.h: $(IDLH)
