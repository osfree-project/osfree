# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

!ifdef HOSTBLD
DEST = none
SRC = $(MYDIR)..$(SEP)..$(SEP)
OS2TK = $(%OS2TK)
SOM_INTDIR=$(BLD)host$(SEP)$(%HOST)$(SEP)$(RELDIR)..$(SEP)som
PLATFORM = host$(SEP)$(%HOST)$(SEP)
!else
DEST = none
SRC = $(MYDIR)..$(SEP)
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45
SOM_INTDIR=$(BLD)$(RELDIR)..$(SEP)som
!endif

SOM_DIR=$(SRC)contrib$(SEP)
SOMIDL_IDL=$(OS2TK)$(SEP)som$(SEP)include
SOMIDL_HEADERS=$(SOMIDL_IDL) #$(SEP)$(%OS)



SOMKERNP_KIH=$(SOM_INTDIR)$(SEP)somkernp.kih
SOM_IDL=$(SOM_DIR)somkpub$(SEP)som$(SEP)somobj.idl  $(SOM_DIR)somkpub$(SEP)som$(SEP)somcls.idl  $(SOM_DIR)somkpub$(SEP)som$(SEP)somcm.idl

SOM_HEADERS=	$(SOM_INTDIR)$(SEP)somobj.h		&
				$(SOM_INTDIR)$(SEP)somcls.h		&
				$(SOM_INTDIR)$(SEP)somcm.h		&
				$(SOM_INTDIR)$(SEP)somobj.ih		&
				$(SOM_INTDIR)$(SEP)somcls.ih		&
				$(SOM_INTDIR)$(SEP)somcm.ih		&
				$(SOM_INTDIR)$(SEP)rhbsomuk.h

!include $(%ROOT)tools/mk/all.mk

TARGETS = pre $(SOMKERNP_KIH) $(SOM_HEADERS)

pre: .symbolic
 @$(MDHIER) $(SOM_INTDIR)

ADD_SCOPT=-p -I$(SOM_DIR)somkpub$(SEP)som -I$(SOMIDL_IDL)

$(SOMKERNP_KIH): $(SOM_DIR)som$(SEP)somkernp.idl
 $(verbose)$(BLD)bin$(SEP)host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)bin$(SEP)$(SC) -skih -o $(SOM_INTDIR) $(SOM_DIR)som$(SEP)somkernp.idl -D __GENERATE_SOM__  $(ADD_SCOPT)

$(SOM_HEADERS): $(SOM_IDL) $(SOM_DIR)somcdr$(SEP)rhbsomuk.idl

.idl: $(SOM_DIR)somkpub$(SEP)som;$(SOM_DIR)somir;$(SOM_DIR)somd;$(SOM_DIR)somestrm;$(SOM_DIR)somabs1;$(SOM_DIR)somnmf;$(SOM_DIR)somdcomm;$(SOM_DIR)somos;$(SOM_DIR)somu;$(SOM_DIR)somu2;$(SOM_DIR)somcslib;$(SOM_DIR)soms;$(SOM_DIR)somem;$(SOM_DIR)somst
.h: $(ROOT)build$(SEP)include$(SEP)somidl$(SEP)$(%OS)
.ih: $(ROOT)build$(SEP)include$(SEP)somidl$(SEP)$(%OS)
