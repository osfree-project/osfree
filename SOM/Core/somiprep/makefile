# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

SOM_DIR=$(MYDIR)..$(SEP)contrib$(SEP)
SOMIDL_IDL=$(%OS2TK)$(SEP)som$(SEP)include
SOMIDL_HEADERS=$(SOMIDL_IDL)

SOMTC_INTDIR=$(BLD)host$(SEP)$(%HOST)$(SEP)$(RELDIR)..$(SEP)somtc
SOMREF_INTDIR=$(BLD)host$(SEP)$(%HOST)$(SEP)$(RELDIR)..$(SEP)somref
SOMIR_INTDIR=$(BLD)host$(SEP)$(%HOST)$(SEP)$(RELDIR)..$(SEP)somir


SOMREF_H=	$(SOMIDL_HEADERS)$(SEP)somref.h	&
			$(SOMIDL_HEADERS)$(SEP)somref.xh

SOMREF_IH=	$(SOMREF_INTDIR)$(SEP)somref.h &
			$(SOMREF_INTDIR)$(SEP)somref.ih
			

SOMREF_PDL=	$(SOMIDL_IDL)$(SEP)somref.idl

SOMIR_IH=	$(SOMIR_INTDIR)$(SEP)containd.ih	&
			$(SOMIR_INTDIR)$(SEP)containd.h	&
			$(SOMIR_INTDIR)$(SEP)containr.ih	&
			$(SOMIR_INTDIR)$(SEP)containr.h	&
			$(SOMIR_INTDIR)$(SEP)typedef.ih	&
			$(SOMIR_INTDIR)$(SEP)typedef.h	&
			$(SOMIR_INTDIR)$(SEP)repostry.ih	&
			$(SOMIR_INTDIR)$(SEP)repostry.h	&
			$(SOMIR_INTDIR)$(SEP)excptdef.ih	&
			$(SOMIR_INTDIR)$(SEP)excptdef.h	&
			$(SOMIR_INTDIR)$(SEP)constdef.ih	&
			$(SOMIR_INTDIR)$(SEP)constdef.h	&
			$(SOMIR_INTDIR)$(SEP)operatdf.ih	&
			$(SOMIR_INTDIR)$(SEP)operatdf.h	&
			$(SOMIR_INTDIR)$(SEP)intfacdf.ih	&
			$(SOMIR_INTDIR)$(SEP)intfacdf.h	&
			$(SOMIR_INTDIR)$(SEP)moduledf.ih	&
			$(SOMIR_INTDIR)$(SEP)moduledf.h	&
			$(SOMIR_INTDIR)$(SEP)paramdef.ih	&
			$(SOMIR_INTDIR)$(SEP)paramdef.h	&
			$(SOMIR_INTDIR)$(SEP)attribdf.ih	&
			$(SOMIR_INTDIR)$(SEP)attribdf.h

SOMIR_1=	$(SOMIDL_IDL)$(SEP)somobj.idl		&
			$(SOMIDL_IDL)$(SEP)somcls.idl		&
			$(SOMIDL_IDL)$(SEP)somcm.idl

SOMIR_2=	$(SOMIDL_IDL)$(SEP)somref.idl		

SOMIR_3=	$(SOMIDL_IDL)$(SEP)intfacdf.idl		&
			$(SOMIDL_IDL)$(SEP)moduledf.idl		&
			$(SOMIDL_IDL)$(SEP)excptdef.idl		&
			$(SOMIDL_IDL)$(SEP)attribdf.idl		&
			$(SOMIDL_IDL)$(SEP)operatdf.idl		&
			$(SOMIDL_IDL)$(SEP)typedef.idl		&
			$(SOMIDL_IDL)$(SEP)paramdef.idl		&
			$(SOMIDL_IDL)$(SEP)constdef.idl		&
			$(SOMIDL_IDL)$(SEP)repostry.idl

!include $(%ROOT)/mk/all.mk

MKDIRS=$(SOMTC_INTDIR) $(SOMREF_INTDIR) $(SOMIR_INTDIR)

TARGETS=$(MKDIRS) $(SOMREF_IH) $(SOMTCDAT_TC) $(SOMREF_PDL) $(SOMREF_H) $(SOMIR_IH) $(SOMNEWIR)

$(MKDIRS): .symbolic
  @$(MDHIER) $@

SOMTCDAT_TC=$(SOMTC_INTDIR)$(SEP)somtcdat.tc

$(SOMTCDAT_TC): $(SOM_DIR)somtc$(SEP)somtcdat.idl
 @$(SAY) SC       $^. $(LOG)
 $(verbose)$(%OS2TK)$(SEP)som$(SEP)bin$(SEP)sc.exe -stc $< -o $^: -p -D __GENERATE_SOMTC__ -I$(SOMIDL_IDL)

#$(SOMREF_H): $(SOMREF_PDL)

$(SOMREF_INTDIR)$(SEP)somref.ih: $(SOM_DIR)somref$(SEP)somref.idl
 @$(SAY) SC       $^. $(LOG)
 $(verbose)$(%OS2TK)$(SEP)som$(SEP)bin$(SEP)sc.exe -sih $< -o $^: -p -I$(SOMIDL_IDL)

$(SOMREF_INTDIR)$(SEP)somref.h: $(SOM_DIR)somref$(SEP)somref.idl
 @$(SAY) SC       $^. $(LOG)
 $(verbose)$(%OS2TK)$(SEP)som$(SEP)bin$(SEP)sc.exe -sh $< -o $^: -p  -I$(SOMIDL_IDL)

$(SOMREF_PDL): $(SOM_DIR)somref$(SEP)somref.idl
 @$(SAY) PDL      $^. $(LOG)
 @pdl.exe -d $(SOMIDL_IDL) $<

$(SOMIDL_HEADERS)$(SEP)somref.h: $(SOMIDL_IDL)$(SEP)somref.idl
 @$(SAY) SC       $^. $(LOG)
 $(verbose)$(%OS2TK)$(SEP)som$(SEP)bin$(SEP)sc.exe -sh $< -o $^: -I$(SOMIDL_IDL)

$(SOMIDL_HEADERS)$(SEP)somref.xh: $(SOMIDL_IDL)$(SEP)somref.idl
 @$(SAY) SC       $^. $(LOG)
 $(verbose)$(%OS2TK)$(SEP)som$(SEP)bin$(SEP)sc.exe -sxh $< -o $^: -I$(SOMIDL_IDL)

#ADD_SCOPT=-p -I$(SOM_DIR)somir -I$(SOMIDL_IDL)

.idl: $(SOM_DIR)somir
.h: $(SOMIR_INTDIR);$(SOMIDL_HEADERS)
.ih: $(SOMIR_INTDIR)

#-----------------------------------------------------------
#
#SC_SOMIR=$(SC_DIR)/som.ir
#SOMNEWIR=$(OUTDIR_ETC)/somnew.ir
#
#$(SOMNEWIR): $(SOMIR_1) $(SOMIR_2) $(SOMIR_3)  $(SC_SOMIR)
#	if test ! -f $@; then cp $(SC_SOMIR) $@; fi
#	for e in $(SOMIR_1) $(SOMIR_2) $(SOMIR_3); \
#	do \
#		SOMIR=$(SOMNEWIR) $(SC) -sir -u -I $(SOMIDL_IDL) $$e; \
#	done


