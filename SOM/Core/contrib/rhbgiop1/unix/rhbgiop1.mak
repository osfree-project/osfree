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

SC=$(HOSTDIR_BIN)/sc$(EXEEXT)

SOMIR=$(OUTDIR_ETC)/som.ir
SOMIDL_IDL=../../somidl
SOMDD_DIR=../../somdd
SOMD_DIR=../../somd
SC_DIR=../../sc

SOMD_INTDIR=$(SOMD_DIR)/$(PLATFORM)/$(BUILDTYPE)
SC_SOMIR=$(SC_DIR)/som.ir

IPV6TEST_H=../../ipv6test/$(PLATFORM)/$(BUILDTYPE)/ipv6test.h

DIRS=	$(SOMD_INTDIR)			

all: $(DIRS) $(IPV6TEST_H) $(SOMIR)

clean:
	$(CLEAN) $(RHBGIOP1_IH)
	$(CLEAN) $(IPV6TEST_H)

SOMIR_1=	$(SOMIDL_IDL)/somobj.idl		\
			$(SOMIDL_IDL)/somcls.idl		\
			$(SOMIDL_IDL)/somcm.idl

SOMIR_2=	$(SOMIDL_IDL)/somref.idl		\
			$(SOMIDL_IDL)/corba.idl			\
			$(SOMIDL_IDL)/dynany.idl		\
			$(SOMIDL_IDL)/poa.idl			\
			$(SOMDD_DIR)/somddsrv.idl

SOMIR_3=	$(SOMIDL_IDL)/intfacdf.idl		\
			$(SOMIDL_IDL)/moduledf.idl		\
			$(SOMIDL_IDL)/excptdef.idl		\
			$(SOMIDL_IDL)/attribdf.idl		\
			$(SOMIDL_IDL)/operatdf.idl		\
			$(SOMIDL_IDL)/typedef.idl		\
			$(SOMIDL_IDL)/paramdef.idl		\
			$(SOMIDL_IDL)/constdef.idl		\
			$(SOMIDL_IDL)/repostry.idl

SOMIR_4=	$(SOMIDL_IDL)/stexcep.idl		\
			$(SOMIDL_IDL)/somdserv.idl		\
			$(SOMIDL_IDL)/somdcprx.idl		\
			$(SOMIDL_IDL)/somoa.idl			\
			$(SOMIDL_IDL)/implrep.idl		\
			$(SOMIDL_IDL)/orb.idl			\
			$(SOMIDL_IDL)/somestio.idl		\
			$(SOMIDL_IDL)/tcpsock.idl

$(SOMIR): $(SOMIR_1) $(SOMIR_2) $(SOMIR_3) $(SOMIR_4) $(SC_SOMIR)
	if test ! -f $@; then cp $(SC_SOMIR) $@; fi
	for e in $(SOMIR_1) $(SOMIR_2) $(SOMIR_3) $(SOMIR_4); \
	do \
		SOMIR=$(SOMIR) $(SC) -sir -u -I $(SOMIDL_IDL) $$e; \
	done

dist install:

