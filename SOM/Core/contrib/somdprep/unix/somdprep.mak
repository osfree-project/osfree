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

IDLTOOL=../../toolbox/idltool.sh
SC=$(HOSTDIR_BIN)/sc
PDL=$(HOSTDIR_BIN)/pdl$(EXEEXT)
SOMIDL_IDL=../../somidl
SOMIDL_HEADERS=$(SOMIDL_IDL)/$(PLATFORM)

SOM_DIR=../../somkpub/som
SOMD_DIR=../../somd
SOMCORBA_DIR=../../somcorba
SOMANY_DIR=../../somany
SOMU_DIR=../../somu
SOMU2_DIR=../../somu2
SOMABS1_DIR=../../somabs1
SOMEM_DIR=../../somem
SOMS_DIR=../../soms
SOMST_DIR=../../somst
SOMDPREP_DIR=../../somdprep
SOMESTRM_DIR=../../somestrm
SOMDCOMM_DIR=../../somdcomm
SOMOS_DIR=../../somos
SOMCDR_DIR=../../somcdr
SOMCSLIB_DIR=../../somcslib
SOMNMF_DIR=../../somnmf
SOMP_DIR=../../somp

SOMD_INTDIR=$(SOMD_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMCORBA_INTDIR=$(SOMCORBA_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMANY_INTDIR=$(SOMANY_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMU_INTDIR=$(SOMU_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMU2_INTDIR=$(SOMU2_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMABS1_INTDIR=$(SOMABS1_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMEM_INTDIR=$(SOMEM_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMS_INTDIR=$(SOMS_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMST_INTDIR=$(SOMST_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMESTRM_INTDIR=$(SOMESTRM_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMDCOMM_INTDIR=$(SOMDCOMM_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMOS_INTDIR=$(SOMOS_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMCDR_INTDIR=$(SOMCDR_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMCSLIB_INTDIR=$(SOMCSLIB_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMNMF_INTDIR=$(SOMNMF_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMP_INTDIR=$(SOMP_DIR)/$(PLATFORM)/$(BUILDTYPE)

SOMDTCDT_TC=$(SOMD_INTDIR)/somdtcdt.tc
SOMDTCDT_IDL=$(SOMD_DIR)/somdtcdt.idl
SOMDINCL1=-I$(SOMD_DIR) -I$(SOM_DIR) -I$(SOMIDL_IDL)
SOMDINCL2=-I$(SOMD_DIR) -I$(SOMIDL_IDL)

PUBLIC_IDLS=$(SOMIDL_IDL)/dynany.idl		\
			$(SOMIDL_IDL)/somcdr.idl		\
			$(SOMIDL_IDL)/poa.idl			\
			$(SOMIDL_IDL)/corba.idl			\
			$(SOMIDL_IDL)/servreq.idl		\
			$(SOMIDL_IDL)/somperrd.idl		\
			$(SOMIDL_IDL)/somperr.idl		\
			$(SOMIDL_IDL)/po.idl

SOMDTYPE_IH=$(SOMD_INTDIR)/somdtype.ih		\
			$(SOMD_INTDIR)/somdtype.h

SOMD_IH1=	$(SOMD_INTDIR)/rhbtypes.h		\
			$(SOMD_INTDIR)/orb.ih			\
			$(SOMD_INTDIR)/orb.h			\
			$(SOMD_INTDIR)/somdcprx.ih		\
			$(SOMD_INTDIR)/somdcprx.h		\
			$(SOMD_INTDIR)/somdobj.ih		\
			$(SOMD_INTDIR)/somdobj.h

SOMD_IH2=	$(SOMD_INTDIR)/cntxt.ih			\
			$(SOMD_INTDIR)/cntxt.h			\
			$(SOMD_INTDIR)/om.ih			\
			$(SOMD_INTDIR)/om.h				\
			$(SOMD_INTDIR)/somdom.ih		\
			$(SOMD_INTDIR)/somdom.h			\
			$(SOMD_INTDIR)/somdserv.ih		\
			$(SOMD_INTDIR)/somdserv.h		\
			$(SOMD_INTDIR)/servmgr.ih		\
			$(SOMD_INTDIR)/servmgr.h		

SOMD_IH3=	$(SOMD_INTDIR)/impldef.ih		\
			$(SOMD_INTDIR)/impldef.h		\
			$(SOMD_INTDIR)/implrep.ih		\
			$(SOMD_INTDIR)/implrep.h		\
			$(SOMD_INTDIR)/somdmprx.h		\
			$(SOMD_INTDIR)/somdmprx.ih		\
			$(SOMD_INTDIR)/poa.h			\
			$(SOMD_INTDIR)/poa.ih			\
			$(SOMD_INTDIR)/servreq.h		\
			$(SOMD_INTDIR)/servreq.ih		\
			$(SOMD_INTDIR)/somdfenc.h		\
			$(SOMD_INTDIR)/somdfenc.ih		\
			$(SOMD_INTDIR)/rhborbst.h		\
			$(SOMD_INTDIR)/rhborbst.ih

SOMD_IH4=	$(SOMD_INTDIR)/principl.ih		\
			$(SOMD_INTDIR)/principl.h		\
			$(SOMD_INTDIR)/somdstub.ih		\
			$(SOMD_INTDIR)/somdstub.h		\
			$(SOMD_INTDIR)/unotypes.ih		\
			$(SOMD_INTDIR)/unotypes.h		\
			$(SOMD_INTDIR)/boa.ih			\
			$(SOMD_INTDIR)/boa.h			\
			$(SOMD_INTDIR)/somoa.ih			\
			$(SOMD_INTDIR)/somoa.h			\
			$(SOMD_INTDIR)/request.ih		\
			$(SOMD_INTDIR)/request.h		\
			$(SOMD_INTDIR)/nvlist.ih		\
			$(SOMD_INTDIR)/nvlist.h			

SOMDCOMM_IH=$(SOMDCOMM_INTDIR)/formarsh.h	\
			$(SOMDCOMM_INTDIR)/formarsh.ih	\
			$(SOMDCOMM_INTDIR)/rhbsomus.h	\
			$(SOMDCOMM_INTDIR)/rhbsomus.ih

SOMCORBA_IH=$(SOMCORBA_INTDIR)/corba.h		\
			$(SOMCORBA_INTDIR)/corba.ih

SOMCORBA_H= $(SOMIDL_HEADERS)/corba.h		\
			$(SOMIDL_HEADERS)/corba.xh

SOMANY_H=	$(SOMIDL_HEADERS)/dynany.h		\
			$(SOMIDL_HEADERS)/dynany.xh

SOMNMF_H=	$(SOMIDL_HEADERS)/naming.h		\
			$(SOMIDL_HEADERS)/naming.xh

SOMANY_IH=	$(SOMANY_INTDIR)/dynany.h		\
			$(SOMANY_INTDIR)/dynany.ih

SOMABS1_IH=	$(SOMABS1_INTDIR)/omgidobj.h	\
			$(SOMABS1_INTDIR)/omgidobj.ih

SOMU_IH=	$(SOMU_INTDIR)/somsid.h			\
			$(SOMU_INTDIR)/somsid.ih		\
			$(SOMU_INTDIR)/somida.h			\
			$(SOMU_INTDIR)/somida.ih		\
			$(SOMU_INTDIR)/somproxy.h		\
			$(SOMU_INTDIR)/somproxy.ih		\
			$(SOMU_INTDIR)/snglicls.h		\
			$(SOMU_INTDIR)/snglicls.ih		\
			$(SOMU_INTDIR)/sombacls.h		\
			$(SOMU_INTDIR)/sombacls.ih

SOMS_IH=	$(SOMS_INTDIR)/somssock.h		\
			$(SOMS_INTDIR)/somssock.ih

SOMST_IH=	$(SOMST_INTDIR)/tcpsock.h		\
			$(SOMST_INTDIR)/tcpsock.ih

SOMEM_IH=	$(SOMEM_INTDIR)/clientev.h		\
			$(SOMEM_INTDIR)/clientev.ih		\
			$(SOMEM_INTDIR)/eman.h			\
			$(SOMEM_INTDIR)/eman.ih			\
			$(SOMEM_INTDIR)/emregdat.h		\
			$(SOMEM_INTDIR)/emregdat.ih		\
			$(SOMEM_INTDIR)/event.h			\
			$(SOMEM_INTDIR)/event.ih		\
			$(SOMEM_INTDIR)/sinkev.h		\
			$(SOMEM_INTDIR)/sinkev.ih		\
			$(SOMEM_INTDIR)/workprev.h		\
			$(SOMEM_INTDIR)/workprev.ih		\
			$(SOMEM_INTDIR)/timerev.h		\
			$(SOMEM_INTDIR)/timerev.ih

SOMU2_IH=	$(SOMU2_INTDIR)/somtdm.h		\
			$(SOMU2_INTDIR)/somtdm.ih

SOMESTRM_IH=$(SOMESTRM_INTDIR)/omgestio.h	\
			$(SOMESTRM_INTDIR)/omgestio.ih	\
			$(SOMESTRM_INTDIR)/somestio.h	\
			$(SOMESTRM_INTDIR)/somestio.ih

SOMCDR_IH=	$(SOMCDR_INTDIR)/somcdr.h		\
			$(SOMCDR_INTDIR)/somcdr.ih	\
			$(SOMCDR_INTDIR)/rhbsomuk.h		\
			$(SOMCDR_INTDIR)/rhbsomuk.ih

SOMCDR_H=	$(SOMIDL_HEADERS)/somcdr.h	\
			$(SOMIDL_HEADERS)/somcdr.xh	

SOMD_H=		$(SOMIDL_HEADERS)/poa.h			\
			$(SOMIDL_HEADERS)/poa.xh		\
			$(SOMIDL_HEADERS)/servreq.h		\
			$(SOMIDL_HEADERS)/servreq.xh	\
			$(SOMIDL_HEADERS)/po.h			\
			$(SOMIDL_HEADERS)/po.xh			\
			$(SOMIDL_HEADERS)/somperrd.h	\
			$(SOMIDL_HEADERS)/somperr.h		\
			$(SOMIDL_HEADERS)/somperrd.xh	\
			$(SOMIDL_HEADERS)/somperr.xh	

SOMD_IH=$(SOMD_IH2) $(SOMD_IH3) $(SOMD_IH4)

SOMOS_IH=	$(SOMOS_INTDIR)/somos.ih		\
			$(SOMOS_INTDIR)/somos.h			\
			$(SOMOS_INTDIR)/somap.ih		\
			$(SOMOS_INTDIR)/somap.h

SOMCSLIB_IH=$(SOMCSLIB_INTDIR)/xmscssae.h	\
			$(SOMCSLIB_INTDIR)/xmscssae.ih

SOMNMF_IH=	$(SOMNMF_INTDIR)/biter.h		\
			$(SOMNMF_INTDIR)/biter.ih		\
			$(SOMNMF_INTDIR)/naming.h		\
			$(SOMNMF_INTDIR)/naming.ih		\
			$(SOMNMF_INTDIR)/xnaming.h		\
			$(SOMNMF_INTDIR)/xnaming.ih		\
			$(SOMNMF_INTDIR)/xnamingf.h		\
			$(SOMNMF_INTDIR)/xnamingf.ih	\
			$(SOMNMF_INTDIR)/lname.h		\
			$(SOMNMF_INTDIR)/lname.ih		\
			$(SOMNMF_INTDIR)/lnamec.h		\
			$(SOMNMF_INTDIR)/lnamec.ih	

SOMP_IH=	$(SOMP_INTDIR)/defedidl.h		\
			$(SOMP_INTDIR)/eda.h			\
			$(SOMP_INTDIR)/fma.h			\
			$(SOMP_INTDIR)/fmi.h			\
			$(SOMP_INTDIR)/fmib.h			\
			$(SOMP_INTDIR)/fsagm.h			\
			$(SOMP_INTDIR)/fsgm.h			\
			$(SOMP_INTDIR)/iogma.h			\
			$(SOMP_INTDIR)/iogrp.h			\
			$(SOMP_INTDIR)/mia.h			\
			$(SOMP_INTDIR)/nsma.h			\
			$(SOMP_INTDIR)/objset.h			\
			$(SOMP_INTDIR)/pid.h			\
			$(SOMP_INTDIR)/po.h				\
			$(SOMP_INTDIR)/poid.h			\
			$(SOMP_INTDIR)/poida.h			\
			$(SOMP_INTDIR)/psma.h			\
			$(SOMP_INTDIR)/somp.h			\
			$(SOMP_INTDIR)/somperr.h		\
			$(SOMP_INTDIR)/somperrd.h		\
			$(SOMP_INTDIR)/sompstad.h		\
			$(SOMP_INTDIR)/sompstat.h		\
			$(SOMP_INTDIR)/srkset.h			\
			$(SOMP_INTDIR)/defedidl.ih		\
			$(SOMP_INTDIR)/eda.ih			\
			$(SOMP_INTDIR)/fma.ih			\
			$(SOMP_INTDIR)/fmi.ih			\
			$(SOMP_INTDIR)/fmib.ih			\
			$(SOMP_INTDIR)/fsagm.ih			\
			$(SOMP_INTDIR)/fsgm.ih			\
			$(SOMP_INTDIR)/iogma.ih			\
			$(SOMP_INTDIR)/iogrp.ih			\
			$(SOMP_INTDIR)/mia.ih			\
			$(SOMP_INTDIR)/nsma.ih			\
			$(SOMP_INTDIR)/objset.ih		\
			$(SOMP_INTDIR)/pid.ih			\
			$(SOMP_INTDIR)/po.ih			\
			$(SOMP_INTDIR)/poid.ih			\
			$(SOMP_INTDIR)/poida.ih			\
			$(SOMP_INTDIR)/psma.ih			\
			$(SOMP_INTDIR)/somp.ih			\
			$(SOMP_INTDIR)/somperr.ih		\
			$(SOMP_INTDIR)/somperrd.ih		\
			$(SOMP_INTDIR)/sompstad.ih		\
			$(SOMP_INTDIR)/sompstat.ih		\
			$(SOMP_INTDIR)/srkset.ih

DIRS=	$(SOMD_INTDIR)			\
		$(SOMCORBA_INTDIR)		\
		$(SOMANY_INTDIR)		\
		$(SOMABS1_INTDIR)		\
		$(SOMU_INTDIR)			\
		$(SOMS_INTDIR)			\
		$(SOMST_INTDIR)			\
		$(SOMEM_INTDIR)			\
		$(SOMU2_INTDIR)			\
		$(SOMESTRM_INTDIR)		\
		$(SOMDCOMM_INTDIR)		\
		$(SOMCDR_INTDIR)		\
		$(SOMOS_INTDIR)			\
		$(SOMCSLIB_INTDIR)		\
		$(SOMNMF_INTDIR)		\
		$(SOMP_INTDIR)

all: 	$(DIRS)			\
		$(PUBLIC_IDLS)	\
		$(SOMDTCDT_TC)	\
		$(SOMD_IH1)		\
		$(SOMD_IH)		\
		$(SOMANY_IH)	\
		$(SOMANY_H)	\
		$(SOMCORBA_H)	\
		$(SOMCORBA_IH)	\
		$(SOMABS1_IH)	\
		$(SOMU_IH)		\
		$(SOMS_IH)		\
		$(SOMST_IH)		\
		$(SOMEM_IH)		\
		$(SOMU2_IH)		\
		$(SOMESTRM_IH)	\
		$(SOMDCOMM_IH)	\
		$(SOMCDR_IH)	\
		$(SOMCDR_H)		\
		$(SOMD_H)		\
		$(SOMDTYPE_IH)	\
		$(SOMOS_IH)		\
		$(SOMCSLIB_IH)	\
		$(SOMNMF_IH)	\
		$(SOMNMF_H)		\
		$(SOMP_IH)

$(SOMDTCDT_TC): $(SC) $(SOMDTCDT_IDL)
	$(IDLTOOL) $(SC) $(SOMD_DIR) -o $@ -p -D__GENERATE_SOMD__ -I$(SOMD_DIR) -I$(SOMIDL_IDL)

clean:
	$(CLEAN) $(SOMDTCDT_TC)
	$(CLEAN) $(SOMD_IH1)
	$(CLEAN) $(SOMD_IH2)
	$(CLEAN) $(SOMD_IH3)
	$(CLEAN) $(SOMD_IH4)
	$(CLEAN) $(SOMCORBA_IH)
	$(CLEAN) $(SOMCORBA_H)
	$(CLEAN) $(SOMANY_IH)
	$(CLEAN) $(SOMANY_H)
	$(CLEAN) $(PUBLIC_IDLS)
	$(CLEAN) $(SOMABS1_IH)
	$(CLEAN) $(SOMU_IH)
	$(CLEAN) $(SOMS_IH)
	$(CLEAN) $(SOMST_IH)
	$(CLEAN) $(SOMEM_IH)
	$(CLEAN) $(SOMU2_IH)
	$(CLEAN) $(SOMESTRM_IH)
	$(CLEAN) $(SOMDCOMM_IH)
	$(CLEAN) $(SOMCDR_IH)
	$(CLEAN) $(SOMCDR_H)
	$(CLEAN) $(SOMD_H)
	$(CLEAN) $(SOMDTYPE_IH)
	$(CLEAN) $(SOMOS_IH)
	$(CLEAN) $(SOMCSLIB_IH)
	$(CLEAN) $(SOMNMF_IH)
	$(CLEAN) $(SOMNMF_H)
	$(CLEAN) $(SOMP_IH)
	 
$(SOMD_IH1) : $(SOMD_DIR)/somdcprx.idl $(SOMD_DIR)/somdobj.idl $(SOM_DIR)/somobj.idl
	$(IDLTOOL) $(SC) $(SOMD_DIR) -o $@ -p $(SOMDINCL1)

$(SOMD_IH) : $(SOMD_DIR)/somdcprx.idl $(SOMD_DIR)/somdobj.idl $(SOM_DIR)/somobj.idl
	$(IDLTOOL) $(SC) $(SOMD_DIR) -o $@ -p $(SOMDINCL2)

$(SOMDTYPE_IH) : $(SOMD_DIR)/somdtype.idl
	$(IDLTOOL) $(SC) $(SOMD_DIR) -o $@ -p $(SOMDINCL2) -DEMIT_SOMDTYPES

$(PUBLIC_IDLS):
	$(IDLTOOL) $(PDL)		\
		$(SOMCORBA_DIR)		\
		$(SOMANY_DIR)		\
		$(SOMEM_DIR)		\
		$(SOMD_DIR)			\
		$(SOMABS1_DIR)		\
		$(SOMCDR_DIR)		\
		$(SOMP_DIR)			\
		-o $@

$(SOMANY_IH): 
	$(IDLTOOL) $(SC) $(SOMANY_DIR)	\
					-o $@ -p -I$(SOMANY_DIR) -I$(SOMIDL_IDL)

$(SOMOS_IH): 
	$(IDLTOOL) $(SC) $(SOMOS_DIR)	\
					-o $@ -p -I$(SOMOS_DIR) -I$(SOMIDL_IDL)

$(SOMCORBA_IH): 
	$(IDLTOOL) $(SC) $(SOMCORBA_DIR)		\
					-o $@ -p -I$(SOMCORBA_DIR) -I$(SOMIDL_IDL)

$(SOMCORBA_H) $(SOMANY_H) $(SOMCDR_H) $(SOMD_H): $(PUBLIC_IDLS)
	$(IDLTOOL) $(SC) $(SOMIDL_IDL)		\
					-o $@ -I$(SOMIDL_IDL)

$(SOMNMF_H): $(PUBLIC_IDLS)
	$(IDLTOOL) $(SC) $(SOMIDL_IDL)		\
					-o $@ -I$(SOMIDL_IDL)

$(SOMABS1_IH): 
	$(IDLTOOL) $(SC) $(SOMABS1_DIR)	\
					-o $@ -p -I$(SOMABS1_DIR) -I$(SOMIDL_IDL)

$(SOMU_IH):  
	$(IDLTOOL) $(SC) $(SOMU_DIR)	\
					-o $@ -p -I$(SOMU_DIR) -I$(SOMIDL_IDL)

$(SOMS_IH):  
	$(IDLTOOL) $(SC) $(SOMS_DIR)	\
					-o $@ -p -I$(SOMS_DIR) -I$(SOMIDL_IDL)

$(SOMEM_IH):  
	$(IDLTOOL) $(SC) $(SOMEM_DIR)	\
					-o $@ -p -I$(SOMEM_DIR) -I$(SOMIDL_IDL)

$(SOMST_IH): 
	$(IDLTOOL) $(SC) $(SOMST_DIR)	\
					-o $@ -p -I$(SOMST_DIR) -I$(SOMIDL_IDL)

$(SOMU2_IH): 
	$(IDLTOOL) $(SC) $(SOMU2_DIR)	\
					-o $@ -p -I$(SOMU2_DIR) -I$(SOMIDL_IDL)

$(SOMESTRM_IH): 
	$(IDLTOOL) $(SC) $(SOMESTRM_DIR)	\
					-o $@ -p -I$(SOMESTRM_DIR) -I$(SOMIDL_IDL)

$(SOMCDR_IH): 
	$(IDLTOOL) $(SC) $(SOMCDR_DIR)	\
					-o $@ -p -I$(SOMCDR_DIR) -I$(SOMIDL_IDL)

$(SOMDCOMM_IH): 
	$(IDLTOOL) $(SC) $(SOMDCOMM_DIR)	\
					-o $@ -p -I$(SOMDCOMM_DIR) -I$(SOMIDL_IDL)

$(SOMCSLIB_IH):  
	$(IDLTOOL) $(SC) $(SOMCSLIB_DIR)	\
					-o $@ -p -I$(SOMCSLIB_DIR) -I$(SOMIDL_IDL)

$(SOMNMF_IH): 
	$(IDLTOOL) $(SC) $(SOMNMF_DIR)	\
					-o $@ -p -I$(SOMNMF_DIR) -I$(SOMIDL_IDL)

$(SOMP_IH):  
	$(IDLTOOL) $(SC) $(SOMP_DIR)	\
					-o $@ -p -I$(SOMP_DIR) -I$(SOMIDL_IDL)

$(DIRS):
	while test ! -d $@; 			\
	do NAME=$@; 					\
		while test ! -d $$NAME; 	\
		do PARENT=`dirname $$NAME`; \
			if test -d $$PARENT;	\
			then					\
				mkdir $$NAME;		\
				if test "$$?" != 0;	\
				then exit 1; 		\
				fi;					\
			else					\
				NAME=$$PARENT; 		\
			fi; 					\
		done; 						\
	done




dist install:

