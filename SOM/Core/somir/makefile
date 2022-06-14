# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

TARGETS=$(ROOT)build$(SEP)include$(SEP)attribdf.h &
$(ROOT)build$(SEP)include$(SEP)constdef.h &
$(ROOT)build$(SEP)include$(SEP)containd.h &
$(ROOT)build$(SEP)include$(SEP)containr.h &
$(ROOT)build$(SEP)include$(SEP)excptdef.h &
$(ROOT)build$(SEP)include$(SEP)intfacdf.h &
$(ROOT)build$(SEP)include$(SEP)moduledf.h &
$(ROOT)build$(SEP)include$(SEP)operatdf.h &
$(ROOT)build$(SEP)include$(SEP)paramdef.h &
$(ROOT)build$(SEP)include$(SEP)repostry.h &
$(ROOT)build$(SEP)include$(SEP)typedef.h

ADD_SCOPT = -D__PRIVATE__ &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somd &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somir &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somref

!include $(%ROOT)/mk/all.mk

.idl: $(MYDIR)..$(SEP)contrib$(SEP)somir
.h: $(ROOT)build$(SEP)include
