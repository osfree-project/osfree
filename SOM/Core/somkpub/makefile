# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

SOM_DIR=..$(SEP)contrib$(SEP)somkpub$(SEP)som
SOM_IDL=$(SOM_DIR)$(SEP)somobj.idl $(SOM_DIR)$(SEP)somcls.idl $(SOM_DIR)$(SEP)somcm.idl
SOMIDL_IDL=$(ROOT)build$(SEP)include$(SEP)somidl
SOMIDL_HEADERS=$(SOMIDL_IDL)$(SEP)$(%OS)

PRODUCTS_SC=	$(SOMIDL_HEADERS)$(SEP)somobj.h	&
				$(SOMIDL_HEADERS)$(SEP)somobj.xh	&
				$(SOMIDL_HEADERS)$(SEP)somcls.h	&
				$(SOMIDL_HEADERS)$(SEP)somcls.xh	&
				$(SOMIDL_HEADERS)$(SEP)somcm.h	&
				$(SOMIDL_HEADERS)$(SEP)somcm.xh	&
				$(SOMIDL_HEADERS)$(SEP)somcls.api

PRODUCTS_PDL=	$(SOMIDL_IDL)$(SEP)somobj.idl	&
				$(SOMIDL_IDL)$(SEP)somcls.idl	&
				$(SOMIDL_IDL)$(SEP)somcm.idl


ADD_SCOPT = -I$(SOMIDL_IDL)

!include $(%ROOT)/mk/all.mk

TARGETS=$(PRODUCTS_PDL) $(PRODUCTS_SC)

$(PRODUCTS_PDL): $(SOM_IDL)
  pdl -d $(SOMIDL_IDL) $<

.idl: $(MYDIR)..$(SEP)contrib$(SEP)somkpub$(SEP)som
.h: $(ROOT)build$(SEP)include$(SEP)somidl$(SEP)$(%OS)
.xh: $(ROOT)build$(SEP)include$(SEP)somidl$(SEP)$(%OS)

