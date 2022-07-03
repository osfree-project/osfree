# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

!ifdef HOSTBLD
SRC = $(MYDIR)..$(SEP)..$(SEP)
OS2TK = $(%OS2TK)
PLATFORM = host$(SEP)$(%HOST)$(SEP)
!else
SRC = $(MYDIR)..$(SEP)
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45
!endif

SOM_DIR=$(SRC)contrib$(SEP)
SOMIDL_IDL=$(OS2TK)$(SEP)som$(SEP)include
SOMIDL_HEADERS=$(SOMIDL_IDL) #$(SEP)$(%OS)

SOM_IDL=		$(SOM_DIR)somir$(SEP)containd.idl		&
				$(SOM_DIR)somir$(SEP)containr.idl		&
				$(SOM_DIR)somir$(SEP)paramdef.idl		&
				$(SOM_DIR)somir$(SEP)excptdef.idl		&
				$(SOM_DIR)somir$(SEP)constdef.idl		&
				$(SOM_DIR)somir$(SEP)operatdf.idl		&
				$(SOM_DIR)somir$(SEP)attribdf.idl		&
				$(SOM_DIR)somir$(SEP)intfacdf.idl		&
				$(SOM_DIR)somir$(SEP)repostry.idl		&
				$(SOM_DIR)somir$(SEP)moduledf.idl		&
				$(SOM_DIR)somd$(SEP)somdtype.idl		&
				$(SOM_DIR)somd$(SEP)stexcep.idl			&
				$(SOM_DIR)somu$(SEP)somproxy.idl		&
				$(SOM_DIR)somd$(SEP)somdcprx.idl		&
				$(SOM_DIR)somd$(SEP)somdobj.idl			&
				$(SOM_DIR)somd$(SEP)unotypes.idl		&
				$(SOM_DIR)somd$(SEP)somdserv.idl		&
				$(SOM_DIR)somd$(SEP)orb.idl				&
				$(SOM_DIR)somd$(SEP)nvlist.idl			&
				$(SOM_DIR)somd$(SEP)request.idl			&
				$(SOM_DIR)somd$(SEP)cntxt.idl			&
				$(SOM_DIR)somd$(SEP)om.idl				&
				$(SOM_DIR)somd$(SEP)somdom.idl			&
				$(SOM_DIR)somd$(SEP)boa.idl				&
				$(SOM_DIR)somd$(SEP)somoa.idl			&
				$(SOM_DIR)somd$(SEP)principl.idl		&
				$(SOM_DIR)somd$(SEP)somdserv.idl		&
				$(SOM_DIR)somd$(SEP)impldef.idl			&
				$(SOM_DIR)somd$(SEP)implrep.idl			&
				$(SOM_DIR)somd$(SEP)servmgr.idl			&
				$(SOM_DIR)soms$(SEP)somssock.idl		&
				$(SOM_DIR)somem$(SEP)emregdat.idl		&
				$(SOM_DIR)somcslib$(SEP)xmscssae.idl	&
				$(SOM_DIR)somir$(SEP)typedef.idl &
				$(SOM_DIR)somestrm$(SEP)omgestio.idl &
				$(SOM_DIR)somestrm$(SEP)somestio.idl &
				$(SOM_DIR)somnmf$(SEP)naming.idl &
				$(SOM_DIR)somnmf$(SEP)lname.idl &
				$(SOM_DIR)somnmf$(SEP)lnamec.idl &
				$(SOM_DIR)somnmf$(SEP)xnamingf.idl &
				$(SOM_DIR)somnmf$(SEP)xnaming.idl &
				$(SOM_DIR)somnmf$(SEP)biter.idl &
				$(SOM_DIR)somabs1$(SEP)omgidobj.idl &
				$(SOM_DIR)somdcomm$(SEP)formarsh.idl &
				$(SOM_DIR)somos$(SEP)somos.idl &
				$(SOM_DIR)somos$(SEP)somap.idl &
				$(SOM_DIR)somu$(SEP)somsid.idl &
				$(SOM_DIR)somu$(SEP)somida.idl &
				$(SOM_DIR)somu$(SEP)snglicls.idl &
				$(SOM_DIR)somu2$(SEP)somtdm.idl &
				$(SOM_DIR)somem$(SEP)eman.idl &
				$(SOM_DIR)somem$(SEP)event.idl &
				$(SOM_DIR)somem$(SEP)workprev.idl &
				$(SOM_DIR)somem$(SEP)sinkev.idl &
				$(SOM_DIR)somem$(SEP)timerev.idl &
				$(SOM_DIR)somem$(SEP)clientev.idl &
				$(SOM_DIR)somst$(SEP)tcpsock.idl


PRODUCTS_SC1=	$(SOMIDL_HEADERS)$(SEP)containd.h	&
				$(SOMIDL_HEADERS)$(SEP)containd.xh	&
				$(SOMIDL_HEADERS)$(SEP)containr.h	&
				$(SOMIDL_HEADERS)$(SEP)containr.xh	&
				$(SOMIDL_HEADERS)$(SEP)paramdef.h	&
				$(SOMIDL_HEADERS)$(SEP)paramdef.xh	&
				$(SOMIDL_HEADERS)$(SEP)excptdef.h	&
				$(SOMIDL_HEADERS)$(SEP)excptdef.xh	&
				$(SOMIDL_HEADERS)$(SEP)constdef.h	&
				$(SOMIDL_HEADERS)$(SEP)constdef.xh	&
				$(SOMIDL_HEADERS)$(SEP)typedef.h	&
				$(SOMIDL_HEADERS)$(SEP)typedef.xh	&
				$(SOMIDL_HEADERS)$(SEP)operatdf.h	&
				$(SOMIDL_HEADERS)$(SEP)operatdf.xh	&
				$(SOMIDL_HEADERS)$(SEP)moduledf.h	&
				$(SOMIDL_HEADERS)$(SEP)moduledf.xh	&
				$(SOMIDL_HEADERS)$(SEP)attribdf.h	&
				$(SOMIDL_HEADERS)$(SEP)attribdf.xh	&
				$(SOMIDL_HEADERS)$(SEP)intfacdf.h	&
				$(SOMIDL_HEADERS)$(SEP)intfacdf.xh	&
				$(SOMIDL_HEADERS)$(SEP)repostry.h	&
				$(SOMIDL_HEADERS)$(SEP)repostry.xh	&
				$(SOMIDL_HEADERS)$(SEP)stexcep.h	&
				$(SOMIDL_HEADERS)$(SEP)stexcep.xh	&
				$(SOMIDL_HEADERS)$(SEP)omgestio.xh	&
				$(SOMIDL_HEADERS)$(SEP)omgestio.h	&
				$(SOMIDL_HEADERS)$(SEP)somestio.xh

PRODUCTS_SC2=	$(SOMIDL_HEADERS)$(SEP)somestio.h	&
				$(SOMIDL_HEADERS)$(SEP)omgidobj.xh	&
				$(SOMIDL_HEADERS)$(SEP)omgidobj.h	&
				$(SOMIDL_HEADERS)$(SEP)naming.xh		&
				$(SOMIDL_HEADERS)$(SEP)naming.h		&
				$(SOMIDL_HEADERS)$(SEP)xnaming.xh	&
				$(SOMIDL_HEADERS)$(SEP)xnaming.h		&
				$(SOMIDL_HEADERS)$(SEP)xnamingf.xh	&
				$(SOMIDL_HEADERS)$(SEP)xnamingf.h	&
				$(SOMIDL_HEADERS)$(SEP)formarsh.xh	&
				$(SOMIDL_HEADERS)$(SEP)formarsh.h	&
				$(SOMIDL_HEADERS)$(SEP)somos.xh		&
				$(SOMIDL_HEADERS)$(SEP)somos.h		&
				$(SOMIDL_HEADERS)$(SEP)snglicls.xh	&
				$(SOMIDL_HEADERS)$(SEP)snglicls.h	&
				$(SOMIDL_HEADERS)$(SEP)somdserv.xh	&
				$(SOMIDL_HEADERS)$(SEP)somdserv.h	&
				$(SOMIDL_HEADERS)$(SEP)somdobj.xh	&
				$(SOMIDL_HEADERS)$(SEP)somdobj.h		&
				$(SOMIDL_HEADERS)$(SEP)somproxy.xh	&
				$(SOMIDL_HEADERS)$(SEP)somproxy.h	&
				$(SOMIDL_HEADERS)$(SEP)unotypes.xh	&
				$(SOMIDL_HEADERS)$(SEP)unotypes.h	&
				$(SOMIDL_HEADERS)$(SEP)xmscssae.xh	&
				$(SOMIDL_HEADERS)$(SEP)xmscssae.h	&
				$(SOMIDL_HEADERS)$(SEP)boa.xh		&
				$(SOMIDL_HEADERS)$(SEP)boa.h			&
				$(SOMIDL_HEADERS)$(SEP)somoa.xh		&
				$(SOMIDL_HEADERS)$(SEP)somoa.h		&
				$(SOMIDL_HEADERS)$(SEP)somdom.xh		&
				$(SOMIDL_HEADERS)$(SEP)somdom.h

PRODUCTS_SC3=	$(SOMIDL_HEADERS)$(SEP)nvlist.xh		&
				$(SOMIDL_HEADERS)$(SEP)nvlist.h		&
				$(SOMIDL_HEADERS)$(SEP)request.xh	&
				$(SOMIDL_HEADERS)$(SEP)request.h		&
				$(SOMIDL_HEADERS)$(SEP)orb.xh		&
				$(SOMIDL_HEADERS)$(SEP)orb.h			&
				$(SOMIDL_HEADERS)$(SEP)om.xh			&
				$(SOMIDL_HEADERS)$(SEP)om.h			&
				$(SOMIDL_HEADERS)$(SEP)principl.xh	&
				$(SOMIDL_HEADERS)$(SEP)principl.h	&
				$(SOMIDL_HEADERS)$(SEP)cntxt.xh		&
				$(SOMIDL_HEADERS)$(SEP)cntxt.h		&
				$(SOMIDL_HEADERS)$(SEP)impldef.xh	&
				$(SOMIDL_HEADERS)$(SEP)impldef.h		&
				$(SOMIDL_HEADERS)$(SEP)implrep.xh	&
				$(SOMIDL_HEADERS)$(SEP)implrep.h		&
				$(SOMIDL_HEADERS)$(SEP)servmgr.xh	&
				$(SOMIDL_HEADERS)$(SEP)servmgr.h		&
				$(SOMIDL_HEADERS)$(SEP)somdcprx.xh	&
				$(SOMIDL_HEADERS)$(SEP)somdcprx.h	&
				$(SOMIDL_HEADERS)$(SEP)biter.xh		&
				$(SOMIDL_HEADERS)$(SEP)biter.h		&
				$(SOMIDL_HEADERS)$(SEP)lname.xh		&
				$(SOMIDL_HEADERS)$(SEP)lname.h		&
				$(SOMIDL_HEADERS)$(SEP)lnamec.xh		&
				$(SOMIDL_HEADERS)$(SEP)lnamec.h		&
				$(SOMIDL_HEADERS)$(SEP)somap.xh		&
				$(SOMIDL_HEADERS)$(SEP)somap.h		&
				$(SOMIDL_HEADERS)$(SEP)somtdm.xh		&
				$(SOMIDL_HEADERS)$(SEP)somtdm.h

PRODUCTS_SC4=	$(SOMIDL_HEADERS)$(SEP)somssock.h	&
				$(SOMIDL_HEADERS)$(SEP)somssock.xh	&
				$(SOMIDL_HEADERS)$(SEP)emregdat.h	&
				$(SOMIDL_HEADERS)$(SEP)emregdat.xh	&
				$(SOMIDL_HEADERS)$(SEP)event.h		&
				$(SOMIDL_HEADERS)$(SEP)event.xh		&
				$(SOMIDL_HEADERS)$(SEP)clientev.h	&
				$(SOMIDL_HEADERS)$(SEP)clientev.xh	&
				$(SOMIDL_HEADERS)$(SEP)sinkev.h		&
				$(SOMIDL_HEADERS)$(SEP)sinkev.xh		&
				$(SOMIDL_HEADERS)$(SEP)workprev.h	&
				$(SOMIDL_HEADERS)$(SEP)workprev.xh	&
				$(SOMIDL_HEADERS)$(SEP)timerev.xh	&
				$(SOMIDL_HEADERS)$(SEP)timerev.h		&
				$(SOMIDL_HEADERS)$(SEP)eman.h		&
				$(SOMIDL_HEADERS)$(SEP)eman.xh		&
				$(SOMIDL_HEADERS)$(SEP)somsid.h		&
				$(SOMIDL_HEADERS)$(SEP)somsid.xh		&
				$(SOMIDL_HEADERS)$(SEP)somida.h		&
				$(SOMIDL_HEADERS)$(SEP)somida.xh		&
				$(SOMIDL_HEADERS)$(SEP)tcpsock.h		&
				$(SOMIDL_HEADERS)$(SEP)tcpsock.xh	

SOMDTYPE_SC=	$(SOMIDL_HEADERS)$(SEP)somdtype.h	&
				$(SOMIDL_HEADERS)$(SEP)somdtype.xh

PRODUCTS_PDL=	$(SOMIDL_IDL)$(SEP)containd.idl		&
				$(SOMIDL_IDL)$(SEP)containr.idl		&
				$(SOMIDL_IDL)$(SEP)excptdef.idl		&
				$(SOMIDL_IDL)$(SEP)paramdef.idl		&
				$(SOMIDL_IDL)$(SEP)operatdf.idl		&
				$(SOMIDL_IDL)$(SEP)repostry.idl		&
				$(SOMIDL_IDL)$(SEP)typedef.idl		&
				$(SOMIDL_IDL)$(SEP)constdef.idl		&
				$(SOMIDL_IDL)$(SEP)stexcep.idl		&
				$(SOMIDL_IDL)$(SEP)somdtype.idl		&
				$(SOMIDL_IDL)$(SEP)omgestio.idl		&
				$(SOMIDL_IDL)$(SEP)somestio.idl		&
				$(SOMIDL_IDL)$(SEP)naming.idl		&
				$(SOMIDL_IDL)$(SEP)xnamingf.idl		&
				$(SOMIDL_IDL)$(SEP)xnaming.idl		&
				$(SOMIDL_IDL)$(SEP)biter.idl			&
				$(SOMIDL_IDL)$(SEP)omgidobj.idl		&
				$(SOMIDL_IDL)$(SEP)formarsh.idl		&
				$(SOMIDL_IDL)$(SEP)somos.idl			&
				$(SOMIDL_IDL)$(SEP)somsid.idl		&
				$(SOMIDL_IDL)$(SEP)somida.idl		&
				$(SOMIDL_IDL)$(SEP)somproxy.idl		&
				$(SOMIDL_IDL)$(SEP)somdserv.idl		&
				$(SOMIDL_IDL)$(SEP)servmgr.idl		&
				$(SOMIDL_IDL)$(SEP)somdobj.idl		&
				$(SOMIDL_IDL)$(SEP)snglicls.idl		&
				$(SOMIDL_IDL)$(SEP)unotypes.idl		&
				$(SOMIDL_IDL)$(SEP)somdcprx.idl		&
				$(SOMIDL_IDL)$(SEP)boa.idl			&
				$(SOMIDL_IDL)$(SEP)principl.idl		&
				$(SOMIDL_IDL)$(SEP)orb.idl			&
				$(SOMIDL_IDL)$(SEP)somoa.idl			&
				$(SOMIDL_IDL)$(SEP)attribdf.idl		&
				$(SOMIDL_IDL)$(SEP)intfacdf.idl		&
				$(SOMIDL_IDL)$(SEP)moduledf.idl		&
				$(SOMIDL_IDL)$(SEP)somdom.idl		&
				$(SOMIDL_IDL)$(SEP)om.idl			&
				$(SOMIDL_IDL)$(SEP)request.idl		&
				$(SOMIDL_IDL)$(SEP)cntxt.idl			&
				$(SOMIDL_IDL)$(SEP)impldef.idl		&
				$(SOMIDL_IDL)$(SEP)implrep.idl		&
				$(SOMIDL_IDL)$(SEP)nvlist.idl		&
				$(SOMIDL_IDL)$(SEP)xmscssae.idl		&
				$(SOMIDL_IDL)$(SEP)somap.idl			&
				$(SOMIDL_IDL)$(SEP)somtdm.idl		&
				$(SOMIDL_IDL)$(SEP)lname.idl			&
				$(SOMIDL_IDL)$(SEP)lnamec.idl		&
				$(SOMIDL_IDL)$(SEP)somssock.idl		&
				$(SOMIDL_IDL)$(SEP)emregdat.idl		&
				$(SOMIDL_IDL)$(SEP)eman.idl			&
				$(SOMIDL_IDL)$(SEP)clientev.idl		&
				$(SOMIDL_IDL)$(SEP)event.idl			&
				$(SOMIDL_IDL)$(SEP)workprev.idl		&
				$(SOMIDL_IDL)$(SEP)sinkev.idl		&
				$(SOMIDL_IDL)$(SEP)timerev.idl		&
				$(SOMIDL_IDL)$(SEP)tcpsock.idl

PRODUCTS_SC=	$(PRODUCTS_SC1)					&
				$(PRODUCTS_SC2)					&
				$(PRODUCTS_SC3)					&
				$(PRODUCTS_SC4)



ADD_SCOPT = -I$(SOMIDL_IDL)

!include $(%ROOT)/mk/all.mk

TARGETS: $(PRODUCTS_PDL) $(SOMDTYPE_SC) $(PRODUCTS_SC)

$(PRODUCTS_PDL): $(SOM_IDL)
 @$(SAY) PDL      $^. $(LOG)
 $(verbose)pdl.exe -d $(SOMIDL_IDL) $<

$(SOMIDL_HEADERS)$(SEP)somdtype.h: $(SOMIDL_IDL)$(SEP)somdtype.idl
 @$(SAY) SC       $^. $(LOG)
 $(verbose)sc.exe -sh $< -o $^: $(ADD_SCOPT) -DEMIT_SOMDTYPES

$(SOMIDL_HEADERS)$(SEP)somdtype.xh: $(SOMIDL_IDL)$(SEP)somdtype.idl
 @$(SAY) SC       $^. $(LOG)
 $(verbose)sc.exe -sxh $< -o $^: $(ADD_SCOPT)  -DEMIT_SOMDTYPES

.idl: $(SOM_DIR)somkpub$(SEP)som;$(SOM_DIR)somir;$(SOM_DIR)somd;$(SOM_DIR)somestrm;$(SOM_DIR)somabs1;$(SOM_DIR)somnmf;$(SOM_DIR)somdcomm;$(SOM_DIR)somos;$(SOM_DIR)somu;$(SOM_DIR)somu2;$(SOM_DIR)somcslib;$(SOM_DIR)soms;$(SOM_DIR)somem;$(SOM_DIR)somst
.h: $(SOMIDL_HEADERS)
.xh: $(SOMIDL_HEADERS)
