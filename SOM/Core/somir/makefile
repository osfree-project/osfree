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
            -I$(MYDIR)..$(SEP)contrib$(SEP)somcdr &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somd &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somir &
            -I$(MYDIR)..$(SEP)contrib$(SEP)somref

!include $(%ROOT)/mk/all.mk

.idl: $(MYDIR)..$(SEP)contrib$(SEP)somir
.h: $(ROOT)build$(SEP)include


$(ROOT)build$(SEP)include$(SEP)attribdf.h: $(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)attribdf.idl
$(ROOT)build$(SEP)include$(SEP)constdef.h: $(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)constdef.idl
$(ROOT)build$(SEP)include$(SEP)containd.h: $(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)containd.idl
$(ROOT)build$(SEP)include$(SEP)containr.h: $(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)containr.idl
$(ROOT)build$(SEP)include$(SEP)excptdef.h: $(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)excptdef.idl
$(ROOT)build$(SEP)include$(SEP)intfacdf.h: $(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)intfacdf.idl
$(ROOT)build$(SEP)include$(SEP)moduledf.h: $(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)moduledf.idl
$(ROOT)build$(SEP)include$(SEP)operatdf.h: $(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)operatdf.idl
$(ROOT)build$(SEP)include$(SEP)paramdef.h: $(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)paramdef.idl
$(ROOT)build$(SEP)include$(SEP)repostry.h: $(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)repostry.idl
$(ROOT)build$(SEP)include$(SEP)typedef.h: $(MYDIR)..$(SEP)contrib$(SEP)somir$(SEP)typedef.idl

