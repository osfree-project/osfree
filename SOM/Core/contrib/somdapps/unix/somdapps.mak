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
PDL=$(HOSTDIR_BIN)/pdl$(EXEEXT)
IDLTOOL=../../toolbox/idltool.sh

SOMIDL_DIR=../../somidl
SOMDD_DIR=../../somdd
SOMD_DIR=../../somd
REGIMPL_DIR=../../regimpl
SOMDDMSG_DIR=../../somddmsg
SOMDAPPS_DIR=../../somdapps
SOMDSVRP_DIR=../../somdsvrp

REGIMPL_INTDIR=$(REGIMPL_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMDD_INTDIR=$(SOMDD_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMDDMSG_INTDIR=$(SOMDDMSG_DIR)/$(PLATFORM)/$(BUILDTYPE)
SOMDSVRP_INTDIR=$(SOMDSVRP_DIR)/$(PLATFORM)/$(BUILDTYPE)

SOMDDSRV_IDL=$(SOMDD_DIR)/somddsrv.idl
SOMDDSRV_TC=$(SOMDD_INTDIR)/somddsrv.tc
SOMDDSRV_PDL=$(SOMIDL_DIR)/somddsrv.idl

REGIORTC_TC=$(REGIMPL_INTDIR)/regiortc.tc
REGIORTC_IDL=$(REGIMPL_DIR)/regiortc.idl

SOMDD_IH=	$(SOMDD_INTDIR)/implreps.ih	\
			$(SOMDD_INTDIR)/implreps.h	\
			$(SOMDD_INTDIR)/somddevm.ih	\
			$(SOMDD_INTDIR)/somddevm.h	\
			$(SOMDD_INTDIR)/somddrep.ih	\
			$(SOMDD_INTDIR)/somddrep.h	\
			$(SOMDD_INTDIR)/somddsrv.ih	\
			$(SOMDD_INTDIR)/somddsrv.h

SOMDSVRP_IH=$(SOMDSVRP_INTDIR)/somdsvrp.ih	\
			$(SOMDSVRP_INTDIR)/somdsvrp.h

DIRS=		$(SOMDD_INTDIR)			\
			$(REGIMPL_INTDIR)		\
			$(SOMDDMSG_INTDIR)		\
			$(SOMDSVRP_INTDIR)

all:	$(DIRS)			\
		$(SOMDD_IH)		\
		$(SOMDDSRV_TC)	\
		$(REGIORTC_TC)	\
		$(SOMDSVRP_IH)	\
		$(SOMDDSRV_PDL)

clean:
	$(CLEAN) $(SOMDD_IH)
	$(CLEAN) $(SOMDDSRV_TC)
	$(CLEAN) $(SOMDDSRV_PDL)
	$(CLEAN) $(REGIORTC_TC)
	$(CLEAN) $(SOMDSVRP_IH)

$(SOMDD_IH):
	$(IDLTOOL) $(SC) $(SOMDD_DIR) -o $@ -p -I$(SOMDD_DIR) -I$(SOMIDL_DIR)

$(SOMDDSRV_TC): $(SC) $(SOMDSRV_IDL)
	$(IDLTOOL) $(SC) $(SOMDD_DIR) -o $@ -p -D__GENERATE_SOMDD__  -I$(SOMDD_DIR) -I$(SOMIDL_DIR)

$(REGIORTC_TC): $(SC) $(REGIORTC_IDL)
	$(IDLTOOL) $(SC) $(REGIMPL_DIR) -o $@ -p -D __GENERATE_REGIMPL__ -I$(REGIMPL_DIR) -I$(SOMD_DIR) -I$(SOMIDL_DIR)

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

$(SOMDSVRP_IH):
	$(IDLTOOL) $(SC) $(SOMDSVRP_DIR) -o $@ -p -I$(SOMDSVRP_DIR) -I$(SOMIDL_DIR)

$(SOMDDSRV_PDL): $(SOMDDSRV_IDL)
	$(PDL) -o $(SOMIDL_DIR) $(SOMDDSRV_IDL) 

dist install:

