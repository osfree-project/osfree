#
#  Copyright 2008, Roger Brown
#
#  This file is part of Roger Brown's Toolkit.
#
#  This program is free software: you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
# 
#  This program is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#  more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>
#
#  $Id$

include $(MAKEDEFS)

SC=$(HOSTDIR_BIN)/sc
PDL=$(HOSTDIR_BIN)/pdl$(EXESUFFIX)
IDLTOOL=../../toolbox/idltool.sh

SOM_IDL=		../../somir/containd.idl		\
				../../somir/containr.idl		\
				../../somir/paramdef.idl		\
				../../somir/excptdef.idl		\
				../../somir/constdef.idl		\
				../../somir/operatdf.idl		\
				../../somir/attribdf.idl		\
				../../somir/intfacdf.idl		\
				../../somir/repostry.idl		\
				../../somir/moduledf.idl		\
				../../somd/somdtype.idl			\
				../../somd/stexcep.idl			\
				../../somu/somproxy.idl			\
				../../somd/somdcprx.idl			\
				../../somd/somdobj.idl			\
				../../somd/unotypes.idl			\
				../../somd/somdserv.idl			\
				../../somd/orb.idl				\
				../../somd/nvlist.idl			\
				../../somd/request.idl			\
				../../somd/cntxt.idl			\
				../../somd/om.idl				\
				../../somd/somdom.idl			\
				../../somd/boa.idl				\
				../../somd/somoa.idl			\
				../../somd/principl.idl			\
				../../somd/somdserv.idl			\
				../../somd/impldef.idl			\
				../../somd/implrep.idl			\
				../../somd/servmgr.idl			\
				../../soms/somssock.idl			\
				../../somem/emregdat.idl		\
				../../somcslib/xmscssae.idl

SOMIDL_IDL=../../somidl
SOMIDL_HEADERS=$(SOMIDL_IDL)/$(PLATFORM)

PRODUCTS_SC1=	$(SOMIDL_HEADERS)/containd.h	\
				$(SOMIDL_HEADERS)/containd.xh	\
				$(SOMIDL_HEADERS)/containr.h	\
				$(SOMIDL_HEADERS)/containr.xh	\
				$(SOMIDL_HEADERS)/paramdef.h	\
				$(SOMIDL_HEADERS)/paramdef.xh	\
				$(SOMIDL_HEADERS)/excptdef.h	\
				$(SOMIDL_HEADERS)/excptdef.xh	\
				$(SOMIDL_HEADERS)/constdef.h	\
				$(SOMIDL_HEADERS)/constdef.xh	\
				$(SOMIDL_HEADERS)/typedef.h		\
				$(SOMIDL_HEADERS)/typedef.xh	\
				$(SOMIDL_HEADERS)/operatdf.h	\
				$(SOMIDL_HEADERS)/operatdf.xh	\
				$(SOMIDL_HEADERS)/moduledf.h	\
				$(SOMIDL_HEADERS)/moduledf.xh	\
				$(SOMIDL_HEADERS)/attribdf.h	\
				$(SOMIDL_HEADERS)/attribdf.xh	\
				$(SOMIDL_HEADERS)/intfacdf.h	\
				$(SOMIDL_HEADERS)/intfacdf.xh	\
				$(SOMIDL_HEADERS)/repostry.h	\
				$(SOMIDL_HEADERS)/repostry.xh	\
				$(SOMIDL_HEADERS)/stexcep.h		\
				$(SOMIDL_HEADERS)/stexcep.xh	\
				$(SOMIDL_HEADERS)/omgestio.xh	\
				$(SOMIDL_HEADERS)/omgestio.h	\
				$(SOMIDL_HEADERS)/somestio.xh

PRODUCTS_SC2=	$(SOMIDL_HEADERS)/somestio.h	\
				$(SOMIDL_HEADERS)/omgidobj.xh	\
				$(SOMIDL_HEADERS)/omgidobj.h	\
				$(SOMIDL_HEADERS)/naming.xh		\
				$(SOMIDL_HEADERS)/naming.h		\
				$(SOMIDL_HEADERS)/xnaming.xh	\
				$(SOMIDL_HEADERS)/xnaming.h		\
				$(SOMIDL_HEADERS)/xnamingf.xh	\
				$(SOMIDL_HEADERS)/xnamingf.h	\
				$(SOMIDL_HEADERS)/formarsh.xh	\
				$(SOMIDL_HEADERS)/formarsh.h	\
				$(SOMIDL_HEADERS)/somos.xh		\
				$(SOMIDL_HEADERS)/somos.h		\
				$(SOMIDL_HEADERS)/snglicls.xh	\
				$(SOMIDL_HEADERS)/snglicls.h	\
				$(SOMIDL_HEADERS)/somdserv.xh	\
				$(SOMIDL_HEADERS)/somdserv.h	\
				$(SOMIDL_HEADERS)/somdobj.xh	\
				$(SOMIDL_HEADERS)/somdobj.h		\
				$(SOMIDL_HEADERS)/somproxy.xh	\
				$(SOMIDL_HEADERS)/somproxy.h	\
				$(SOMIDL_HEADERS)/unotypes.xh	\
				$(SOMIDL_HEADERS)/unotypes.h	\
				$(SOMIDL_HEADERS)/xmscssae.xh	\
				$(SOMIDL_HEADERS)/xmscssae.h	\
				$(SOMIDL_HEADERS)/boa.xh		\
				$(SOMIDL_HEADERS)/boa.h			\
				$(SOMIDL_HEADERS)/somoa.xh		\
				$(SOMIDL_HEADERS)/somoa.h		\
				$(SOMIDL_HEADERS)/somdom.xh		\
				$(SOMIDL_HEADERS)/somdom.h

PRODUCTS_SC3=	$(SOMIDL_HEADERS)/nvlist.xh		\
				$(SOMIDL_HEADERS)/nvlist.h		\
				$(SOMIDL_HEADERS)/request.xh	\
				$(SOMIDL_HEADERS)/request.h		\
				$(SOMIDL_HEADERS)/orb.xh		\
				$(SOMIDL_HEADERS)/orb.h			\
				$(SOMIDL_HEADERS)/om.xh			\
				$(SOMIDL_HEADERS)/om.h			\
				$(SOMIDL_HEADERS)/principl.xh	\
				$(SOMIDL_HEADERS)/principl.h	\
				$(SOMIDL_HEADERS)/cntxt.xh		\
				$(SOMIDL_HEADERS)/cntxt.h		\
				$(SOMIDL_HEADERS)/impldef.xh	\
				$(SOMIDL_HEADERS)/impldef.h		\
				$(SOMIDL_HEADERS)/implrep.xh	\
				$(SOMIDL_HEADERS)/implrep.h		\
				$(SOMIDL_HEADERS)/servmgr.xh	\
				$(SOMIDL_HEADERS)/servmgr.h		\
				$(SOMIDL_HEADERS)/somdcprx.xh	\
				$(SOMIDL_HEADERS)/somdcprx.h	\
				$(SOMIDL_HEADERS)/biter.xh		\
				$(SOMIDL_HEADERS)/biter.h		\
				$(SOMIDL_HEADERS)/lname.xh		\
				$(SOMIDL_HEADERS)/lname.h		\
				$(SOMIDL_HEADERS)/lnamec.xh		\
				$(SOMIDL_HEADERS)/lnamec.h		\
				$(SOMIDL_HEADERS)/somap.xh		\
				$(SOMIDL_HEADERS)/somap.h		\
				$(SOMIDL_HEADERS)/somtdm.xh		\
				$(SOMIDL_HEADERS)/somtdm.h

PRODUCTS_SC4=	$(SOMIDL_HEADERS)/somssock.h	\
				$(SOMIDL_HEADERS)/somssock.xh	\
				$(SOMIDL_HEADERS)/emregdat.h	\
				$(SOMIDL_HEADERS)/emregdat.xh	\
				$(SOMIDL_HEADERS)/event.h		\
				$(SOMIDL_HEADERS)/event.xh		\
				$(SOMIDL_HEADERS)/clientev.h	\
				$(SOMIDL_HEADERS)/clientev.xh	\
				$(SOMIDL_HEADERS)/sinkev.h		\
				$(SOMIDL_HEADERS)/sinkev.xh		\
				$(SOMIDL_HEADERS)/workprev.h	\
				$(SOMIDL_HEADERS)/workprev.xh	\
				$(SOMIDL_HEADERS)/timerev.xh	\
				$(SOMIDL_HEADERS)/timerev.h		\
				$(SOMIDL_HEADERS)/eman.h		\
				$(SOMIDL_HEADERS)/eman.xh		\
				$(SOMIDL_HEADERS)/somsid.h		\
				$(SOMIDL_HEADERS)/somsid.xh		\
				$(SOMIDL_HEADERS)/somida.h		\
				$(SOMIDL_HEADERS)/somida.xh		\
				$(SOMIDL_HEADERS)/tcpsock.h		\
				$(SOMIDL_HEADERS)/tcpsock.xh	

SOMDTYPE_SC=	$(SOMIDL_HEADERS)/somdtype.h	\
				$(SOMIDL_HEADERS)/somdtype.xh

PRODUCTS_PDL=	$(SOMIDL_IDL)/containd.idl		\
				$(SOMIDL_IDL)/containr.idl		\
				$(SOMIDL_IDL)/excptdef.idl		\
				$(SOMIDL_IDL)/paramdef.idl		\
				$(SOMIDL_IDL)/operatdf.idl		\
				$(SOMIDL_IDL)/repostry.idl		\
				$(SOMIDL_IDL)/typedef.idl		\
				$(SOMIDL_IDL)/constdef.idl		\
				$(SOMIDL_IDL)/stexcep.idl		\
				$(SOMIDL_IDL)/somdtype.idl		\
				$(SOMIDL_IDL)/omgestio.idl		\
				$(SOMIDL_IDL)/somestio.idl		\
				$(SOMIDL_IDL)/naming.idl		\
				$(SOMIDL_IDL)/xnamingf.idl		\
				$(SOMIDL_IDL)/xnaming.idl		\
				$(SOMIDL_IDL)/biter.idl			\
				$(SOMIDL_IDL)/omgidobj.idl		\
				$(SOMIDL_IDL)/formarsh.idl		\
				$(SOMIDL_IDL)/somos.idl			\
				$(SOMIDL_IDL)/somsid.idl		\
				$(SOMIDL_IDL)/somida.idl		\
				$(SOMIDL_IDL)/somproxy.idl		\
				$(SOMIDL_IDL)/somdserv.idl		\
				$(SOMIDL_IDL)/servmgr.idl		\
				$(SOMIDL_IDL)/somdobj.idl		\
				$(SOMIDL_IDL)/snglicls.idl		\
				$(SOMIDL_IDL)/unotypes.idl		\
				$(SOMIDL_IDL)/somdcprx.idl		\
				$(SOMIDL_IDL)/boa.idl			\
				$(SOMIDL_IDL)/principl.idl		\
				$(SOMIDL_IDL)/orb.idl			\
				$(SOMIDL_IDL)/somoa.idl			\
				$(SOMIDL_IDL)/attribdf.idl		\
				$(SOMIDL_IDL)/intfacdf.idl		\
				$(SOMIDL_IDL)/moduledf.idl		\
				$(SOMIDL_IDL)/somdom.idl		\
				$(SOMIDL_IDL)/om.idl			\
				$(SOMIDL_IDL)/request.idl		\
				$(SOMIDL_IDL)/cntxt.idl			\
				$(SOMIDL_IDL)/impldef.idl		\
				$(SOMIDL_IDL)/implrep.idl		\
				$(SOMIDL_IDL)/nvlist.idl		\
				$(SOMIDL_IDL)/xmscssae.idl		\
				$(SOMIDL_IDL)/somap.idl			\
				$(SOMIDL_IDL)/somtdm.idl		\
				$(SOMIDL_IDL)/lname.idl			\
				$(SOMIDL_IDL)/lnamec.idl		\
				$(SOMIDL_IDL)/somssock.idl		\
				$(SOMIDL_IDL)/emregdat.idl		\
				$(SOMIDL_IDL)/eman.idl			\
				$(SOMIDL_IDL)/clientev.idl		\
				$(SOMIDL_IDL)/event.idl			\
				$(SOMIDL_IDL)/workprev.idl		\
				$(SOMIDL_IDL)/sinkev.idl		\
				$(SOMIDL_IDL)/timerev.idl		\
				$(SOMIDL_IDL)/tcpsock.idl

PRODUCTS_SC=	$(PRODUCTS_SC1)					\
				$(PRODUCTS_SC2)					\
				$(PRODUCTS_SC3)					\
				$(PRODUCTS_SC4)

all: $(SOMIDL_IDL) $(SOMIDL_HEADERS) $(PRODUCTS_PDL) $(PRODUCTS_SC) $(SOMDTYPE_SC)



clean:
	$(CLEAN) $(PRODUCTS_SC1) 
	$(CLEAN) $(PRODUCTS_SC2) 
	$(CLEAN) $(PRODUCTS_SC3) 
	$(CLEAN) $(PRODUCTS_SC4) 
	$(CLEAN) $(PRODUCTS_PDL) 
	$(CLEAN) $(SOMDTYPE_SC)

$(SOMIDL_HEADERS) $(SOMIDL_IDL):
	mkdir $@

$(PRODUCTS_PDL): 
	$(IDLTOOL) $(PDL) ../../somd ../../somabs1			\
			../../somir ../../somestrm ../../somnmf		\
			../../somdcomm ../../somos ../../somu		\
			../../somcslib ../../somu2 ../../soms		\
			../../somem ../../somst	../../somcorba		\
			-o $@

$(PRODUCTS_SC):  $(PRODUCTS_PDL)
	$(IDLTOOL) $(SC) $(SOMIDL_IDL) -o $@ -I$(SOMIDL_IDL)

$(SOMDTYPE_SC): $(PRODUCTS_PDL)
	$(IDLTOOL) $(SC) $(SOMIDL_IDL) -o $@ -I$(SOMIDL_IDL) -DEMIT_SOMDTYPES


dist install:

