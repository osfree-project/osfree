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
SC_DIR=../../sc
PDL=$(HOSTDIR_BIN)/pdl$(EXEEXT)
SC_SOMIR=$(SC_DIR)/som.ir
SOMNEWIR=$(OUTDIR_ETC)/somnew.ir
IDLTOOL=../../toolbox/idltool.sh

SOMIDL_IDL=../../somidl
SOMIDL_HEADERS=../../somidl/$(PLATFORM)

SOMTC_PART=../../somtc
SOMREF_PART=../../somref
SOMIR_PART=../../somir
SOM_IDL_DIR=../../somkpub/som

SOMTC_INTDIR=$(SOMTC_PART)/$(PLATFORM)/$(BUILDTYPE)
SOMREF_INTDIR=$(SOMREF_PART)/$(PLATFORM)/$(BUILDTYPE)
SOMIR_INTDIR=$(SOMIR_PART)/$(PLATFORM)/$(BUILDTYPE)

DIRS=$(SOMTC_INTDIR) $(SOMREF_INTDIR) $(SOMIR_INTDIR)

SOMTCDAT_TC=$(SOMTC_INTDIR)/somtcdat.tc

SOMREF_PDL=	$(SOMIDL_IDL)/somref.idl

SOMREF_H=	$(SOMIDL_HEADERS)/somref.h	\
			$(SOMIDL_HEADERS)/somref.xh

SOMREF_IH=	$(SOMREF_INTDIR)/somref.h	\
			$(SOMREF_INTDIR)/somref.ih

SOMIR_IH=	$(SOMIR_INTDIR)/containd.ih	\
			$(SOMIR_INTDIR)/containd.h	\
			$(SOMIR_INTDIR)/containr.ih	\
			$(SOMIR_INTDIR)/containr.h	\
			$(SOMIR_INTDIR)/typedef.ih	\
			$(SOMIR_INTDIR)/typedef.h	\
			$(SOMIR_INTDIR)/repostry.ih	\
			$(SOMIR_INTDIR)/repostry.h	\
			$(SOMIR_INTDIR)/excptdef.ih	\
			$(SOMIR_INTDIR)/excptdef.h	\
			$(SOMIR_INTDIR)/constdef.ih	\
			$(SOMIR_INTDIR)/constdef.h	\
			$(SOMIR_INTDIR)/operatdf.ih	\
			$(SOMIR_INTDIR)/operatdf.h	\
			$(SOMIR_INTDIR)/intfacdf.ih	\
			$(SOMIR_INTDIR)/intfacdf.h	\
			$(SOMIR_INTDIR)/moduledf.ih	\
			$(SOMIR_INTDIR)/moduledf.h	\
			$(SOMIR_INTDIR)/paramdef.ih	\
			$(SOMIR_INTDIR)/paramdef.h	\
			$(SOMIR_INTDIR)/attribdf.ih	\
			$(SOMIR_INTDIR)/attribdf.h

all: $(DIRS) $(SOMTCDAT_TC) $(SOMREF_H) $(SOMREF_IH) $(SOMREF_PDL) $(SOMIR_IH) $(SOMNEWIR)

clean:
	$(CLEAN) $(SOMTCDAT_TC) 
	$(CLEAN) $(SOMREF_H) 
	$(CLEAN) $(SOMREF_IH) 
	$(CLEAN) $(SOMREF_PDL)
	$(CLEAN) $(SOMIR_IH)
	$(CLEAN) $(SOMNEWIR)


$(SOMTCDAT_TC): $(SOMTC_PART)/somtcdat.idl
	$(IDLTOOL) $(SC) $(SOMTC_PART) -o $@ -p -D __GENERATE_SOMTC__ -I$(SOMIDL_IDL)

$(SOMREF_PDL): $(SOMREF_PART)/somref.idl
	$(IDLTOOL) $(PDL) $(SOMREF_PART) -o $@ 	

$(SOMREF_H): $(SOMREF_PDL)
	$(IDLTOOL) $(SC) $(SOMIDL_IDL) -o $@ -I$(SOMIDL_IDL)

$(SOMREF_IH): $(SOMREF_PDL) 
	$(IDLTOOL) $(SC) $(SOMREF_PART) -o $@ -p -I$(SOM_IDL_DIR) -I$(SOMIDL_IDL)

$(SOMIR_IH): $(SOMREF_PDL)
	$(IDLTOOL) $(SC) $(SOMIR_PART) -o $@ -p -I$(SOMIR_PART) -I$(SOMIDL_IDL)

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

SOMIR_1=	$(SOMIDL_IDL)/somobj.idl		\
			$(SOMIDL_IDL)/somcls.idl		\
			$(SOMIDL_IDL)/somcm.idl

SOMIR_2=	$(SOMIDL_IDL)/somref.idl		

SOMIR_3=	$(SOMIDL_IDL)/intfacdf.idl		\
			$(SOMIDL_IDL)/moduledf.idl		\
			$(SOMIDL_IDL)/excptdef.idl		\
			$(SOMIDL_IDL)/attribdf.idl		\
			$(SOMIDL_IDL)/operatdf.idl		\
			$(SOMIDL_IDL)/typedef.idl		\
			$(SOMIDL_IDL)/paramdef.idl		\
			$(SOMIDL_IDL)/constdef.idl		\
			$(SOMIDL_IDL)/repostry.idl

$(SOMNEWIR): $(SOMIR_1) $(SOMIR_2) $(SOMIR_3)  $(SC_SOMIR)
	if test ! -f $@; then cp $(SC_SOMIR) $@; fi
	for e in $(SOMIR_1) $(SOMIR_2) $(SOMIR_3); \
	do \
		SOMIR=$(SOMNEWIR) $(SC) -sir -u -I $(SOMIDL_IDL) $$e; \
	done

dist install:

