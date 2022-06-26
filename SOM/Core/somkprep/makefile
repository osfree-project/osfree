# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

SOM_DIR=..$(SEP)contrib$(SEP)
SOMIDL_IDL=$(%OS2TK)$(SEP)som$(SEP)include
SOMIDL_HEADERS=$(SOMIDL_IDL) #$(SEP)$(%OS)



SOM_INTDIR=$(PATH)$(SEP)..$(SEP)som
SOMKERNP_KIH=$(SOM_INTDIR)$(SEP)somkernp.kih
SOM_IDL=$(SOM_DIR)somkpub$(SEP)som$(SEP)somobj.idl  $(SOM_DIR)somkpub$(SEP)som$(SEP)somcls.idl  $(SOM_DIR)somkpub$(SEP)som$(SEP)somcm.idl

SOM_HEADERS=	$(SOM_INTDIR)$(SEP)somobj.h		&
				$(SOM_INTDIR)$(SEP)somcls.h		&
				$(SOM_INTDIR)$(SEP)somcm.h		&
				$(SOM_INTDIR)$(SEP)somobj.ih		&
				$(SOM_INTDIR)$(SEP)somcls.ih		&
				$(SOM_INTDIR)$(SEP)somcm.ih		&
				$(SOM_INTDIR)$(SEP)rhbsomuk.h

!include $(%ROOT)/mk/all.mk

TARGETS= $(SOMKERNP_KIH) $(SOM_HEADERS)

ADD_SCOPT=-p -I$(SOM_DIR)somkpub$(SEP)som -I$(SOMIDL_IDL)

$(SOMKERNP_KIH): $(SOM_DIR)som$(SEP)somkernp.idl
 $(verbose)$(%OS2TK)$(SEP)som$(SEP)bin$(SEP)sc.exe -skih -o $(SOM_INTDIR) $(SOM_DIR)som$(SEP)somkernp.idl -D __GENERATE_SOM__  $(ADD_SCOPT)

$(SOM_HEADERS): $(SOM_IDL) $(SOM_DIR)somcdr$(SEP)rhbsomuk.idl

.idl: $(SOM_DIR)somkpub$(SEP)som;$(SOM_DIR)somir;$(SOM_DIR)somd;$(SOM_DIR)somestrm;$(SOM_DIR)somabs1;$(SOM_DIR)somnmf;$(SOM_DIR)somdcomm;$(SOM_DIR)somos;$(SOM_DIR)somu;$(SOM_DIR)somu2;$(SOM_DIR)somcslib;$(SOM_DIR)soms;$(SOM_DIR)somem;$(SOM_DIR)somst
.h: $(ROOT)build$(SEP)include$(SEP)somidl$(SEP)$(%OS)
.ih: $(ROOT)build$(SEP)include$(SEP)somidl$(SEP)$(%OS)
