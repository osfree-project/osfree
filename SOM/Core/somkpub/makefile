# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

!ifdef HOSTBLD
SRC = $(MYDIR)..$(SEP)..$(SEP)
OS2TK = $(%OS2TK)
PLATFORM = host$(SEP)$(%HOST)$(SEP)
!else
SRC = $(MYDIR)..$(SEP)
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45
!endif

SOM_DIR=$(SRC)contrib$(SEP)somkpub$(SEP)som
SOMIDL_IDL=$(OS2TK)$(SEP)som$(SEP)include
SOMIDL_HEADERS=$(SOMIDL_IDL)

SOM_IDL=$(SOM_DIR)$(SEP)somobj.idl $(SOM_DIR)$(SEP)somcls.idl $(SOM_DIR)$(SEP)somcm.idl

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

TARGETS=$(PRODUCTS_PDL) $(PRODUCTS_SC) $(SOMIDL_HEADERS)$(SEP)som.xh $(SOMIDL_HEADERS)$(SEP)som.h $(SOMIDL_HEADERS)$(SEP)somtypes.h $(SOMIDL_HEADERS)$(SEP)sombtype.h &
$(SOMIDL_HEADERS)$(SEP)somnames.h $(SOMIDL_HEADERS)$(SEP)somplatf.h $(SOMIDL_HEADERS)$(SEP)somcdev.h $(SOMIDL_HEADERS)$(SEP)somcorba.h &
$(SOMIDL_HEADERS)$(SEP)somapi.h

$(PRODUCTS_SC): $(SOM_IDL)

$(SOMIDL_HEADERS)$(SEP)som.h: $(SOM_DIR)$(SEP)..$(SEP)include$(SEP)som.h
 $(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(SOMIDL_HEADERS)$(SEP)som.xh: $(SOM_DIR)$(SEP)..$(SEP)include$(SEP)som.xh
 $(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(SOMIDL_HEADERS)$(SEP)somtypes.h: $(SOM_DIR)$(SEP)..$(SEP)include$(SEP)somtypes.h
 $(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(SOMIDL_HEADERS)$(SEP)sombtype.h: $(SOM_DIR)$(SEP)..$(SEP)include$(SEP)sombtype.h
 $(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(SOMIDL_HEADERS)$(SEP)somnames.h: $(SOM_DIR)$(SEP)..$(SEP)include$(SEP)somnames.h
 $(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(SOMIDL_HEADERS)$(SEP)somplatf.h: $(SOM_DIR)$(SEP)..$(SEP)include$(SEP)somplatf.h
 $(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(SOMIDL_HEADERS)$(SEP)somcdev.h: $(SOM_DIR)$(SEP)..$(SEP)include$(SEP)somcdev.h
 $(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(SOMIDL_HEADERS)$(SEP)somcorba.h: $(SOM_DIR)$(SEP)..$(SEP)include$(SEP)somcorba.h
 $(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(SOMIDL_HEADERS)$(SEP)somapi.h: $(SOM_DIR)$(SEP)..$(SEP)include$(SEP)somapi.h
 $(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(PRODUCTS_PDL): $(SOM_IDL)
 @$(SAY) PDL      $^. $(LOG)
 @$(MDHIER) $(SOMIDL_IDL)
 @$(MDHIER) $(SOMIDL_HEADERS)
 @pdl.exe -d $(SOMIDL_IDL) $<

.idl: $(SOM_DIR)
.h: $(SOMIDL_HEADERS)
.xh: $(SOMIDL_HEADERS)
.api: $(SOMIDL_HEADERS)
