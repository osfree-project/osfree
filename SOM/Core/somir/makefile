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

ADD_SCOPT = -I$(MYDIR)..$(SEP)contrib$(SEP)somir &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som

!include $(%ROOT)/mk/all.mk

.idl: $(MYDIR)..$(SEP)contrib$(SEP)somir
.h: $(ROOT)build$(SEP)include
