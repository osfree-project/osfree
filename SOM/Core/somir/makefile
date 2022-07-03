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

!ifdef HOSTBLD
SRC = $(MYDIR)..$(SEP)..$(SEP)
!else
SRC = $(MYDIR)..$(SEP)
!endif

ADD_SCOPT = -D__PRIVATE__ &
            -I$(SRC)contrib$(SEP)somkpub$(SEP)som &
            -I$(SRC)contrib$(SEP)somcdr &
            -I$(SRC)contrib$(SEP)somd &
            -I$(SRC)contrib$(SEP)somir &
            -I$(SRC)contrib$(SEP)somref

!include $(%ROOT)/mk/all.mk

.idl: $(SRC)contrib$(SEP)somir
.h: $(ROOT)build$(SEP)include


$(ROOT)build$(SEP)include$(SEP)attribdf.h: $(SRC)contrib$(SEP)somir$(SEP)attribdf.idl
$(ROOT)build$(SEP)include$(SEP)constdef.h: $(SRC)contrib$(SEP)somir$(SEP)constdef.idl
$(ROOT)build$(SEP)include$(SEP)containd.h: $(SRC)contrib$(SEP)somir$(SEP)containd.idl
$(ROOT)build$(SEP)include$(SEP)containr.h: $(SRC)contrib$(SEP)somir$(SEP)containr.idl
$(ROOT)build$(SEP)include$(SEP)excptdef.h: $(SRC)contrib$(SEP)somir$(SEP)excptdef.idl
$(ROOT)build$(SEP)include$(SEP)intfacdf.h: $(SRC)contrib$(SEP)somir$(SEP)intfacdf.idl
$(ROOT)build$(SEP)include$(SEP)moduledf.h: $(SRC)contrib$(SEP)somir$(SEP)moduledf.idl
$(ROOT)build$(SEP)include$(SEP)operatdf.h: $(SRC)contrib$(SEP)somir$(SEP)operatdf.idl
$(ROOT)build$(SEP)include$(SEP)paramdef.h: $(SRC)contrib$(SEP)somir$(SEP)paramdef.idl
$(ROOT)build$(SEP)include$(SEP)repostry.h: $(SRC)contrib$(SEP)somir$(SEP)repostry.idl
$(ROOT)build$(SEP)include$(SEP)typedef.h: $(SRC)contrib$(SEP)somir$(SEP)typedef.idl
