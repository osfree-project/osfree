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

INTDIR=$(BUILDTYPE)

BUILD=../..
FRAMEWORKS=$(OUTDIR_FRAMEWORKS)
SUBFRAMEWORKS=$(FRAMEWORKS)/SOMTK.framework/Versions/A/Frameworks
PLUGINS=$(FRAMEWORKS)/SOMTK.framework/Versions/A/PlugIns
IMPLIBS=$(INTDIR)/implibs
SHARED=-dynamiclib -Wl,-single_module
BUNDLE=-bundle
OBJDIR=$(INTDIR)/obj
SRCDIR=../src
BINDIR=$(FRAMEWORKS)/SOMTK.framework/Versions/A/bin
STDLIB=-lpthread -lc -L$(BUILD)/products/$(PLATFORM)/$(BUILDTYPE)/lib -lrhbmtuta $(DLLIBS)

all: uuid-$(HAVE_LIBUUID) SOMTK $(BINDIR) $(PLUGINS) $(BINDIR)/irdump $(PLUGINS)/somir.dll $(PLUGINS)/somref.dll $(PLUGINS)/somcslib.dll $(PLUGINS)/somu.dll $(PLUGINS)/somu2.dll $(PLUGINS)/somestrm.dll $(PLUGINS)/somnmf.dll $(PLUGINS)/somdcomm.dll $(PLUGINS)/somcorba.dll $(PLUGINS)/somany.dll $(PLUGINS)/somcdr.dll $(PLUGINS)/somd.dll $(PLUGINS)/somos.dll $(BINDIR)/somdd $(BINDIR)/regimpl $(BINDIR)/somdsvr $(BINDIR)/dsom $(BINDIR)/somossvr $(BINDIR)/somipc $(BINDIR)/somdchk

uuid-true:

uuid-false: UUID

clean:
	rm -rf $(BINDIR) $(IMPLIBS) $(FRAMEWORKS)/SOMTK.framework $(OBJDIR)

$(SUBFRAMEWORKS)/SOM.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMREF.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMTC.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMIR.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMD.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMU.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMU2.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMABS1.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMESTRM.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMOS.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMNMF.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMDCOMM.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMANY.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMCORBA.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMCDR.framework/Versions/A \
$(SUBFRAMEWORKS)/SOMCSLIB.framework/Versions/A \
$(SUBFRAMEWORKS)/UUID.framework/Versions/A \
$(IMPLIBS)/SOMESTRM.framework/Versions/A \
$(IMPLIBS)/SOMOS.framework/Versions/A \
$(IMPLIBS)/SOMNMF.framework/Versions/A \
$(BINDIR) $(OBJDIR) $(PLUGINS):
	mkdir -p $@

SOMTK: $(FRAMEWORKS)/SOMTK.framework/Versions/A/SOMTK $(FRAMEWORKS)/SOMTK.framework/SOMTK

UUID: $(SUBFRAMEWORKS)/UUID.framework/Versions/A/UUID $(SUBFRAMEWORKS)/UUID.framework/UUID

SOM: $(SUBFRAMEWORKS)/SOM.framework/Versions/A/SOM $(SUBFRAMEWORKS)/SOM.framework/SOM

SOMCSLIB: $(SUBFRAMEWORKS)/SOMCSLIB.framework/Versions/A/SOMCSLIB $(SUBFRAMEWORKS)/SOMCSLIB.framework/SOMCSLIB

SOMTC: $(SUBFRAMEWORKS)/SOMTC.framework/Versions/A/SOMTC $(SUBFRAMEWORKS)/SOMTC.framework/SOMTC

SOMIR: $(SUBFRAMEWORKS)/SOMIR.framework/Versions/A/SOMIR $(SUBFRAMEWORKS)/SOMIR.framework/SOMIR

SOMREF: $(SUBFRAMEWORKS)/SOMREF.framework/Versions/A/SOMREF $(SUBFRAMEWORKS)/SOMREF.framework/SOMREF

SOMANY: $(SUBFRAMEWORKS)/SOMANY.framework/Versions/A/SOMANY $(SUBFRAMEWORKS)/SOMANY.framework/SOMANY

SOMCORBA: $(SUBFRAMEWORKS)/SOMCORBA.framework/Versions/A/SOMCORBA $(SUBFRAMEWORKS)/SOMCORBA.framework/SOMCORBA

SOMOS: $(SUBFRAMEWORKS)/SOMOS.framework/Versions/A/SOMOS $(SUBFRAMEWORKS)/SOMOS.framework/SOMOS

SOMCDR: $(SUBFRAMEWORKS)/SOMCDR.framework/Versions/A/SOMCDR $(SUBFRAMEWORKS)/SOMCDR.framework/SOMCDR

SOMNMF: $(SUBFRAMEWORKS)/SOMNMF.framework/Versions/A/SOMNMF $(SUBFRAMEWORKS)/SOMNMF.framework/SOMNMF

SOMESTRM: $(SUBFRAMEWORKS)/SOMESTRM.framework/Versions/A/SOMESTRM $(SUBFRAMEWORKS)/SOMESTRM.framework/SOMESTRM

SOMDCOMM: $(SUBFRAMEWORKS)/SOMDCOMM.framework/Versions/A/SOMDCOMM $(SUBFRAMEWORKS)/SOMDCOMM.framework/SOMDCOMM

SOMD: $(SUBFRAMEWORKS)/SOMD.framework/Versions/A/SOMD $(SUBFRAMEWORKS)/SOMD.framework/SOMD

SOMABS1: $(SUBFRAMEWORKS)/SOMABS1.framework/Versions/A/SOMABS1 $(SUBFRAMEWORKS)/SOMABS1.framework/SOMABS1

SOMU: $(SUBFRAMEWORKS)/SOMU.framework/Versions/A/SOMU $(SUBFRAMEWORKS)/SOMU.framework/SOMU

SOMU2: $(SUBFRAMEWORKS)/SOMU2.framework/Versions/A/SOMU2 $(SUBFRAMEWORKS)/SOMU2.framework/SOMU2

implibs: $(IMPLIBS)/SOMESTRM.framework/SOMESTRM $(IMPLIBS)/SOMOS.framework/SOMOS $(IMPLIBS)/SOMNMF.framework/SOMNMF

$(FRAMEWORKS)/SOMTK.framework/Versions/A/SOMTK: SOM SOMREF SOMTC SOMIR SOMU SOMU2 SOMABS1 SOMANY SOMCORBA implibs SOMCDR SOMNMF SOMESTRM SOMDCOMM SOMD SOMOS SOMCSLIB
	rm -rf $@.*
	for d in `../../toolbox/lipoarch.sh $(SUBFRAMEWORKS)/SOM.framework/SOM`; do \
	echo "const char somtk_arch[]=\"$$d\";" >$(INTDIR)/SOMTK.arch.c; \
	$(CC) -arch $$d 			\
		$(INTDIR)/SOMTK.arch.c \
		`if test "$(PLATFORM_ISYSROOT)" != ""; then echo -isysroot "$(PLATFORM_ISYSROOT)" ; fi` \
		-dynamiclib -o $@.$$d 	\
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/SOMTK \
		-sub_umbrella SOM 		\
		-sub_umbrella SOMTC 	\
		-sub_umbrella SOMIR 	\
		-sub_umbrella SOMREF 	\
		-sub_umbrella SOMU 	\
		-sub_umbrella SOMU2 	\
		-sub_umbrella SOMABS1 	\
		-sub_umbrella SOMNMF 	\
		-sub_umbrella SOMESTRM \
		-sub_umbrella SOMCORBA \
		-sub_umbrella SOMDCOMM	\
		-sub_umbrella SOMANY	\
		-sub_umbrella SOMOS	\
		-sub_umbrella SOMD		\
		-sub_umbrella SOMCDR 	\
		-sub_umbrella SOMCSLIB	\
		-F$(SUBFRAMEWORKS)		\
		-framework SOM			\
		-framework SOMREF		\
		-framework SOMIR		\
		-framework SOMU			\
		-framework SOMU2		\
		-framework SOMABS1		\
		-framework SOMNMF		\
		-framework SOMESTRM		\
		-framework SOMDCOMM		\
		-framework SOMOS		\
		-framework SOMD			\
		-framework SOMCDR		\
		-framework SOMCORBA		\
		-framework SOMANY		\
		-framework SOMCSLIB		\
		`if test -f $(SUBFRAMEWORKS)/UUID.framework/UUID; then echo -framework UUID; fi` \
		$(UUIDLIBS)				\
		-framework SOMTC;		\
		rm $(INTDIR)/SOMTK.arch.c; \
	done
	ls -ld $@.*
	if test `ls $@.* | wc -l` -gt 1; then lipo -create -output $@ $@.*; rm $@.*; lipo -info $@; else mv $@.* $@; fi
	file $@
	lipo -info $@

SOM_OBJS=$(BUILD)/som/$(PLATFORM)/$(BUILDTYPE)/somalloc.o	\
		$(BUILD)/som/$(PLATFORM)/$(BUILDTYPE)/somkern.o		\
		$(BUILD)/som/$(PLATFORM)/$(BUILDTYPE)/sommutex.o	\
		$(BUILD)/som/$(PLATFORM)/$(BUILDTYPE)/somobjva.o	\
		$(BUILD)/som/$(PLATFORM)/$(BUILDTYPE)/somapi.o		\
		$(BUILD)/som/$(PLATFORM)/$(BUILDTYPE)/rhbsomid.o	\
		$(BUILD)/som/$(PLATFORM)/$(BUILDTYPE)/somshlb.o		\
		$(OBJDIR)/somkpath.o

SOMREF_OBJS=$(BUILD)/somref/$(PLATFORM)/$(BUILDTYPE)/somref.o		

SOMCSLIB_OBJS=$(BUILD)/somcslib/$(PLATFORM)/$(BUILDTYPE)/xmscssae.o		

UUID_OBJS=$(BUILD)/uuid/$(PLATFORM)/$(BUILDTYPE)/uuid.o	\
			$(BUILD)/uuid/$(PLATFORM)/$(BUILDTYPE)/rhbenet.o

SOMTC_OBJS=$(BUILD)/somtc/$(PLATFORM)/$(BUILDTYPE)/rhbsomtc.o	\
			$(BUILD)/somtc/$(PLATFORM)/$(BUILDTYPE)/rhbsomva.o 	\
			$(BUILD)/somtc/$(PLATFORM)/$(BUILDTYPE)/rhbsomts.o 

SOMOS_OBJS=$(BUILD)/somos/$(PLATFORM)/$(BUILDTYPE)/rhbsomos.o	\
			$(BUILD)/somos/$(PLATFORM)/$(BUILDTYPE)/somos.o 	\
			$(BUILD)/somos/$(PLATFORM)/$(BUILDTYPE)/somosutl.o 	\
			$(BUILD)/somos/$(PLATFORM)/$(BUILDTYPE)/somap.o 

SOMCORBA_OBJS=$(BUILD)/somcorba/$(PLATFORM)/$(BUILDTYPE)/rhbcorba.o	\
			$(BUILD)/somcorba/$(PLATFORM)/$(BUILDTYPE)/corba.o 

SOMDCOMM_OBJS=$(BUILD)/somdcomm/$(PLATFORM)/$(BUILDTYPE)/formarsh.o	\
			$(BUILD)/somdcomm/$(PLATFORM)/$(BUILDTYPE)/rhbsomus.o \
			$(BUILD)/somdcomm/$(PLATFORM)/$(BUILDTYPE)/somdcomm.o 

SOMANY_OBJS=$(BUILD)/somany/$(PLATFORM)/$(BUILDTYPE)/rhbdynam.o	\
			$(BUILD)/somany/$(PLATFORM)/$(BUILDTYPE)/dynany.o 

SOMCDR_OBJS=$(BUILD)/somcdr/$(PLATFORM)/$(BUILDTYPE)/somcdr.o	\
			$(BUILD)/somcdr/$(PLATFORM)/$(BUILDTYPE)/rhbsomkd.o \
			$(BUILD)/somcdr/$(PLATFORM)/$(BUILDTYPE)/rhbsomue.o \
			$(BUILD)/somcdr/$(PLATFORM)/$(BUILDTYPE)/rhbsomut.o 

SOMESTRM_OBJS=$(BUILD)/somestrm/$(PLATFORM)/$(BUILDTYPE)/rhbestrm.o	\
			$(BUILD)/somestrm/$(PLATFORM)/$(BUILDTYPE)/omgestio.o 	\
			$(BUILD)/somestrm/$(PLATFORM)/$(BUILDTYPE)/somestio.o 

SOMNMF_OBJS=$(BUILD)/somnmf/$(PLATFORM)/$(BUILDTYPE)/biter.o	\
			$(BUILD)/somnmf/$(PLATFORM)/$(BUILDTYPE)/lname.o 	\
			$(BUILD)/somnmf/$(PLATFORM)/$(BUILDTYPE)/naming.o 	\
			$(BUILD)/somnmf/$(PLATFORM)/$(BUILDTYPE)/rhbsomnm.o 	\
			$(BUILD)/somnmf/$(PLATFORM)/$(BUILDTYPE)/xnaming.o 	\
			$(BUILD)/somnmf/$(PLATFORM)/$(BUILDTYPE)/xnamingf.o 	\
			$(BUILD)/somnmf/$(PLATFORM)/$(BUILDTYPE)/lnamec.o 

SOMD_OBJS=$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhbgiop1.o	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhbgiop2.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhbgiop3.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhbgiop4.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhbgiop5.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhbgiop6.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhbgiop7.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhbgiop8.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhbgiop9.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhbgiops.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhborbst.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/request.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/om.o 		\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/nvlist.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/orb.o	 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/poa.o 		\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/principl.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/impldef.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/implrep.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/boa.o 		\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/cntxt.o 		\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/unotypes.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/servmgr.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/servreq.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/somdobj.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/somdcprx.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/somdmprx.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/somdfenc.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/somdom.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/somoa.o 		\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/somdstub.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/somdserv.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/somdtcdt.o 	\
			$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/rhbsomd.o 

SOMIR_OBJS=$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/rhbsomir.o	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/repostry.o 	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/containr.o 	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/typedef.o 	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/paramdef.o 	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/excptdef.o 	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/intfacdf.o 	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/operatdf.o 	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/attribdf.o 	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/constdef.o 	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/moduledf.o 	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/somirfmt.o 	\
			$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/containd.o 

SOMU_OBJS=$(BUILD)/somu/$(PLATFORM)/$(BUILDTYPE)/rhbsomu.o	\
			$(BUILD)/somu/$(PLATFORM)/$(BUILDTYPE)/snglicls.o 	\
			$(BUILD)/somu/$(PLATFORM)/$(BUILDTYPE)/sombacls.o 	\
			$(BUILD)/somu/$(PLATFORM)/$(BUILDTYPE)/somproxy.o 	\
			$(BUILD)/somu/$(PLATFORM)/$(BUILDTYPE)/somida.o 	\
			$(BUILD)/somu/$(PLATFORM)/$(BUILDTYPE)/somsid.o 	\
			$(BUILD)/somu/$(PLATFORM)/$(BUILDTYPE)/rhbiniut.o 	\
			$(BUILD)/somu/$(PLATFORM)/$(BUILDTYPE)/somthrd.o 

SOMU2_OBJS=$(BUILD)/somu2/$(PLATFORM)/$(BUILDTYPE)/rhbsomu2.o	\
			$(BUILD)/somu2/$(PLATFORM)/$(BUILDTYPE)/somtdm.o 

SOMABS1_OBJS=$(BUILD)/somabs1/$(PLATFORM)/$(BUILDTYPE)/somabs1.o	\
			$(BUILD)/somabs1/$(PLATFORM)/$(BUILDTYPE)/omgidobj.o

IRDUMP_OBJS=$(BUILD)/irdump/$(PLATFORM)/$(BUILDTYPE)/irdump.o		

DSOM_OBJS=$(BUILD)/dsom/$(PLATFORM)/$(BUILDTYPE)/dsom.o		

REGIMPL_OBJS=$(BUILD)/regimpl/$(PLATFORM)/$(BUILDTYPE)/regimpl.o \
		$(BUILD)/regimpl/$(PLATFORM)/$(BUILDTYPE)/regior.o \
		$(BUILD)/regimpl/$(PLATFORM)/$(BUILDTYPE)/regiortc.o

SOMDSVR_OBJS=$(BUILD)/somdsvr/$(PLATFORM)/$(BUILDTYPE)/somdsvr-cpp.o 

SOMDCHK_OBJS=$(BUILD)/somdchk/$(PLATFORM)/$(BUILDTYPE)/somdchk.o 

SOMOSSVR_OBJS=$(BUILD)/somossvr/$(PLATFORM)/$(BUILDTYPE)/somossvr-cpp.o \
		$(BUILD)/somossvr/$(PLATFORM)/$(BUILDTYPE)/somosrun.o

SOMDD_OBJS=$(BUILD)/somdd/$(PLATFORM)/$(BUILDTYPE)/rhbsomdd.o \
		$(BUILD)/somdd/$(PLATFORM)/$(BUILDTYPE)/implreps.o \
		$(BUILD)/somdd/$(PLATFORM)/$(BUILDTYPE)/rhbexecd.o \
		$(BUILD)/somdd/$(PLATFORM)/$(BUILDTYPE)/somddior.o \
		$(BUILD)/somdd/$(PLATFORM)/$(BUILDTYPE)/somddrep.o \
		$(BUILD)/somdd/$(PLATFORM)/$(BUILDTYPE)/somddsrv.o \
		$(BUILD)/somdd/$(PLATFORM)/$(BUILDTYPE)/somddtc.o \
		$(BUILD)/somdd/$(PLATFORM)/$(BUILDTYPE)/rhbexec.o

$(FRAMEWORKS)/SOMTK.framework/SOMTK: $(FRAMEWORKS)/SOMTK.framework/Versions/A/SOMTK
	cd $(FRAMEWORKS)/SOMTK.framework; ln -s Versions/A/SOMTK SOMTK

$(SUBFRAMEWORKS)/SOM.framework/SOM: $(SUBFRAMEWORKS)/SOM.framework/Versions/A/SOM
	cd $(SUBFRAMEWORKS)/SOM.framework; ln -s Versions/A/SOM SOM

$(SUBFRAMEWORKS)/SOMREF.framework/SOMREF: $(SUBFRAMEWORKS)/SOMREF.framework/Versions/A/SOMREF
	cd $(SUBFRAMEWORKS)/SOMREF.framework; ln -s Versions/A/SOMREF SOMREF

$(SUBFRAMEWORKS)/SOMCSLIB.framework/SOMCSLIB: $(SUBFRAMEWORKS)/SOMCSLIB.framework/Versions/A/SOMCSLIB
	cd $(SUBFRAMEWORKS)/SOMCSLIB.framework; ln -s Versions/A/SOMCSLIB SOMCSLIB

$(SUBFRAMEWORKS)/SOMTC.framework/SOMTC: $(SUBFRAMEWORKS)/SOMTC.framework/Versions/A/SOMTC
	cd $(SUBFRAMEWORKS)/SOMTC.framework; ln -s Versions/A/SOMTC SOMTC

$(SUBFRAMEWORKS)/UUID.framework/UUID: $(SUBFRAMEWORKS)/UUID.framework/Versions/A/UUID
	cd $(SUBFRAMEWORKS)/UUID.framework; ln -s Versions/A/UUID UUID

$(SUBFRAMEWORKS)/SOMIR.framework/SOMIR: $(SUBFRAMEWORKS)/SOMIR.framework/Versions/A/SOMIR
	cd $(SUBFRAMEWORKS)/SOMIR.framework; ln -s Versions/A/SOMIR SOMIR

$(SUBFRAMEWORKS)/SOMU.framework/SOMU: $(SUBFRAMEWORKS)/SOMU.framework/Versions/A/SOMU
	cd $(SUBFRAMEWORKS)/SOMU.framework; ln -s Versions/A/SOMU SOMU

$(SUBFRAMEWORKS)/SOMU2.framework/SOMU2: $(SUBFRAMEWORKS)/SOMU2.framework/Versions/A/SOMU2
	cd $(SUBFRAMEWORKS)/SOMU2.framework; ln -s Versions/A/SOMU2 SOMU2

$(SUBFRAMEWORKS)/SOMABS1.framework/SOMABS1: $(SUBFRAMEWORKS)/SOMABS1.framework/Versions/A/SOMABS1
	cd $(SUBFRAMEWORKS)/SOMABS1.framework; ln -s Versions/A/SOMABS1 SOMABS1

$(SUBFRAMEWORKS)/SOMDCOMM.framework/SOMDCOMM: $(SUBFRAMEWORKS)/SOMDCOMM.framework/Versions/A/SOMDCOMM
	cd $(SUBFRAMEWORKS)/SOMDCOMM.framework; ln -s Versions/A/SOMDCOMM SOMDCOMM

$(SUBFRAMEWORKS)/SOMD.framework/SOMD: $(SUBFRAMEWORKS)/SOMD.framework/Versions/A/SOMD
	cd $(SUBFRAMEWORKS)/SOMD.framework; ln -s Versions/A/SOMD SOMD

$(SUBFRAMEWORKS)/SOMNMF.framework/SOMNMF: $(SUBFRAMEWORKS)/SOMNMF.framework/Versions/A/SOMNMF
	cd $(SUBFRAMEWORKS)/SOMNMF.framework; ln -s Versions/A/SOMNMF SOMNMF

$(SUBFRAMEWORKS)/SOMOS.framework/SOMOS: $(SUBFRAMEWORKS)/SOMOS.framework/Versions/A/SOMOS
	cd $(SUBFRAMEWORKS)/SOMOS.framework; ln -s Versions/A/SOMOS SOMOS

$(SUBFRAMEWORKS)/SOMESTRM.framework/SOMESTRM: $(SUBFRAMEWORKS)/SOMESTRM.framework/Versions/A/SOMESTRM
	cd $(SUBFRAMEWORKS)/SOMESTRM.framework; ln -s Versions/A/SOMESTRM SOMESTRM

$(SUBFRAMEWORKS)/SOMANY.framework/SOMANY: $(SUBFRAMEWORKS)/SOMANY.framework/Versions/A/SOMANY
	cd $(SUBFRAMEWORKS)/SOMANY.framework; ln -s Versions/A/SOMANY SOMANY

$(SUBFRAMEWORKS)/SOMCORBA.framework/SOMCORBA: $(SUBFRAMEWORKS)/SOMCORBA.framework/Versions/A/SOMCORBA
	cd $(SUBFRAMEWORKS)/SOMCORBA.framework; ln -s Versions/A/SOMCORBA SOMCORBA

$(SUBFRAMEWORKS)/SOMCDR.framework/SOMCDR: $(SUBFRAMEWORKS)/SOMCDR.framework/Versions/A/SOMCDR
	cd $(SUBFRAMEWORKS)/SOMCDR.framework; ln -s Versions/A/SOMCDR SOMCDR

$(SUBFRAMEWORKS)/SOM.framework/Versions/A/SOM: $(SOM_OBJS) $(SUBFRAMEWORKS)/SOM.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOM_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/som/$(PLATFORM)/$(BUILDTYPE)/som.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOM.framework/Versions/A/SOM \
		$(STDLIB)

$(SUBFRAMEWORKS)/UUID.framework/Versions/A/UUID: $(UUID_OBJS) $(SUBFRAMEWORKS)/UUID.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(UUID_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/uuid/$(PLATFORM)/$(BUILDTYPE)/uuid.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/UUID.framework/Versions/A/UUID \
		$(STDLIB)

$(IMPLIBS)/SOMESTRM.framework/SOMESTRM \
$(IMPLIBS)/SOMOS.framework/SOMOS \
$(IMPLIBS)/SOMNMF.framework/SOMNMF: \
	$(IMPLIBS)/SOMESTRM.framework/Versions/A/SOMESTRM \
	$(IMPLIBS)/SOMNMF.framework/Versions/A/SOMNMF \
	$(IMPLIBS)/SOMOS.framework/Versions/A/SOMOS
	cd `dirname $@`; if test ! -f `basename $@`; then ln -s Versions/A/`basename $@`; fi

$(SUBFRAMEWORKS)/SOMREF.framework/Versions/A/SOMREF: $(SOMREF_OBJS) $(SUBFRAMEWORKS)/SOMREF.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMREF_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somref/$(PLATFORM)/$(BUILDTYPE)/somref.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMREF.framework/Versions/A/SOMREF \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM

$(SUBFRAMEWORKS)/SOMCSLIB.framework/Versions/A/SOMCSLIB: $(SOMCSLIB_OBJS) $(SUBFRAMEWORKS)/SOMCSLIB.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMCSLIB_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somcslib/$(PLATFORM)/$(BUILDTYPE)/somcslib.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMCSLIB.framework/Versions/A/SOMCSLIB \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMREF

$(SUBFRAMEWORKS)/SOMANY.framework/Versions/A/SOMANY: $(SOMANY_OBJS) $(SUBFRAMEWORKS)/SOMANY.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMANY_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somany/$(PLATFORM)/$(BUILDTYPE)/somany.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMANY.framework/Versions/A/SOMANY \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMTC

$(SUBFRAMEWORKS)/SOMD.framework/Versions/A/SOMD: $(SOMD_OBJS) $(SUBFRAMEWORKS)/SOMD.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMD_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/somd.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMD.framework/Versions/A/SOMD \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMREF -framework SOMTC -framework SOMDCOMM -framework SOMCDR \
			-framework SOMANY -framework SOMCORBA -framework SOMESTRM -framework SOMU -framework SOMNMF -framework SOMABS1 \
			-F$(IMPLIBS) -framework SOMOS \
			`if test -f $(SUBFRAMEWORKS)/UUID.framework/UUID; then echo -framework UUID; fi` $(UUIDLIBS)

$(SUBFRAMEWORKS)/SOMTC.framework/Versions/A/SOMTC: $(SOMTC_OBJS) $(SUBFRAMEWORKS)/SOMTC.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMTC_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somtc/$(PLATFORM)/$(BUILDTYPE)/somtc.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMTC.framework/Versions/A/SOMTC \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM

$(SUBFRAMEWORKS)/SOMOS.framework/Versions/A/SOMOS: $(SOMOS_OBJS) $(SUBFRAMEWORKS)/SOMOS.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMOS_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somos/$(PLATFORM)/$(BUILDTYPE)/somos.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMOS.framework/Versions/A/SOMOS \
		$(STDLIB) -F$(IMPLIBS) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMTC -framework SOMREF -framework SOMD -framework SOMABS1 \
			-framework SOMU -framework SOMCDR -framework SOMCORBA -framework SOMANY -framework SOMDCOMM \
			-framework SOMU2 -framework SOMNMF -framework SOMESTRM \
			`if test -f $(SUBFRAMEWORKS)/UUID.framework/UUID; then echo -framework UUID; fi` $(UUIDLIBS)

$(SUBFRAMEWORKS)/SOMNMF.framework/Versions/A/SOMNMF: $(SOMNMF_OBJS) $(SUBFRAMEWORKS)/SOMNMF.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMNMF_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somnmf/$(PLATFORM)/$(BUILDTYPE)/somnmf.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMNMF.framework/Versions/A/SOMNMF \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMREF -F$(IMPLIBS) -framework SOMOS

$(SUBFRAMEWORKS)/SOMESTRM.framework/Versions/A/SOMESTRM: $(SOMESTRM_OBJS) $(SUBFRAMEWORKS)/SOMESTRM.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMESTRM_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somestrm/$(PLATFORM)/$(BUILDTYPE)/somestrm.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMESTRM.framework/Versions/A/SOMESTRM \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMREF -framework SOMABS1 -F$(IMPLIBS) -framework SOMOS

$(SUBFRAMEWORKS)/SOMIR.framework/Versions/A/SOMIR: $(SOMIR_OBJS) $(SUBFRAMEWORKS)/SOMIR.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMIR_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/somir.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMIR.framework/Versions/A/SOMIR \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMTC -framework SOMREF

$(SUBFRAMEWORKS)/SOMU.framework/Versions/A/SOMU: $(SOMU_OBJS) $(SUBFRAMEWORKS)/SOMU.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMU_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somu/$(PLATFORM)/$(BUILDTYPE)/somu.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMU.framework/Versions/A/SOMU \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMREF \
			`if test -f $(SUBFRAMEWORKS)/UUID.framework/UUID; then echo -framework UUID; fi` $(UUIDLIBS)

$(SUBFRAMEWORKS)/SOMU2.framework/Versions/A/SOMU2: $(SOMU2_OBJS) $(SUBFRAMEWORKS)/SOMU2.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMU2_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somu2/$(PLATFORM)/$(BUILDTYPE)/somu2.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMU2.framework/Versions/A/SOMU2 \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMREF

$(SUBFRAMEWORKS)/SOMABS1.framework/Versions/A/SOMABS1: $(SOMABS1_OBJS) $(SUBFRAMEWORKS)/SOMABS1.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMABS1_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somabs1/$(PLATFORM)/$(BUILDTYPE)/somabs1.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMABS1.framework/Versions/A/SOMABS1 \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMREF

$(SUBFRAMEWORKS)/SOMCORBA.framework/Versions/A/SOMCORBA: $(SOMCORBA_OBJS) $(SUBFRAMEWORKS)/SOMCORBA.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMCORBA_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somcorba/$(PLATFORM)/$(BUILDTYPE)/somcorba.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMCORBA.framework/Versions/A/SOMCORBA \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMREF

$(SUBFRAMEWORKS)/SOMDCOMM.framework/Versions/A/SOMDCOMM: $(SOMDCOMM_OBJS) $(SUBFRAMEWORKS)/SOMDCOMM.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMDCOMM_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somdcomm/$(PLATFORM)/$(BUILDTYPE)/somdcomm.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMDCOMM.framework/Versions/A/SOMDCOMM \
		$(STDLIB) -F$(SUBFRAMEWORKS) -framework SOM -framework SOMREF -framework SOMCDR \
			-framework SOMTC -framework SOMCORBA -framework SOMESTRM \
			-framework SOMABS1 -F$(IMPLIBS) -framework SOMOS

$(SUBFRAMEWORKS)/SOMCDR.framework/Versions/A/SOMCDR: $(SOMCDR_OBJS) $(SUBFRAMEWORKS)/SOMCDR.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(SOMCDR_OBJS) \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somcdr/$(PLATFORM)/$(BUILDTYPE)/somcdr.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMCDR.framework/Versions/A/SOMCDR \
		$(STDLIB) \
		-F$(SUBFRAMEWORKS) -framework SOM -framework SOMREF -framework SOMTC -framework SOMCORBA 

$(IMPLIBS)/SOMESTRM.framework/Versions/A/SOMESTRM: $(BUILD)/somdstub/$(PLATFORM)/$(BUILDTYPE)/somestrm.o $(IMPLIBS)/SOMESTRM.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(BUILD)/somdstub/$(PLATFORM)/$(BUILDTYPE)/somestrm.o \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somdstub/$(PLATFORM)/$(BUILDTYPE)/somestrm.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMESTRM.framework/Versions/A/SOMESTRM \
		$(STDLIB)

$(IMPLIBS)/SOMNMF.framework/Versions/A/SOMNMF: $(BUILD)/somdstub/$(PLATFORM)/$(BUILDTYPE)/somnmf.o $(IMPLIBS)/SOMNMF.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(BUILD)/somdstub/$(PLATFORM)/$(BUILDTYPE)/somnmf.o \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somdstub/$(PLATFORM)/$(BUILDTYPE)/somnmf.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMNMF.framework/Versions/A/SOMNMF \
		$(STDLIB)

$(IMPLIBS)/SOMOS.framework/Versions/A/SOMOS: $(BUILD)/somdstub/$(PLATFORM)/$(BUILDTYPE)/somos.o $(IMPLIBS)/SOMOS.framework/Versions/A
	$(CC) $(CFLAGS) $(SHARED) -o $@ \
		$(BUILD)/somdstub/$(PLATFORM)/$(BUILDTYPE)/somos.o \
		-umbrella SOMTK \
		-Wl,-exported_symbols_list,$(BUILD)/somdstub/$(PLATFORM)/$(BUILDTYPE)/somos.exp.def \
		-Wl,-install_name,/Library/Frameworks/SOMTK.framework/Versions/A/Frameworks/SOMOS.framework/Versions/A/SOMOS \
		$(STDLIB)

$(BINDIR)/irdump: $(IRDUMP_OBJS)
	$(CC) $(IRDUMP_OBJS) $(CFLAGS) -o $@ $(STDLIB) -F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK 

$(BINDIR)/somdd: $(SOMDD_OBJS)
	$(CC) $(SOMDD_OBJS) $(CFLAGS) -o $@ $(STDLIB) -F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(BINDIR)/somossvr: $(SOMOSSVR_OBJS)
	$(CXX) $(SOMOSSVR_OBJS) $(CXXFLAGS) -o $@ $(STDLIB) -F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(BINDIR)/somdsvr: $(SOMDSVR_OBJS)
	$(CXX) $(SOMDSVR_OBJS) $(CXXFLAGS) -o $@ $(STDLIB) -F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(BINDIR)/somdchk: $(SOMDCHK_OBJS)
	$(CXX) $(SOMDCHK_OBJS) $(CFLAGS) -o $@ $(STDLIB) -F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(BINDIR)/regimpl: $(REGIMPL_OBJS)
	$(CC) $(REGIMPL_OBJS) $(CFLAGS) -o $@ $(STDLIB) -F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(BINDIR)/dsom: $(DSOM_OBJS)
	$(CC) $(DSOM_OBJS) $(CFLAGS) -o $@ $(STDLIB) -F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(PLUGINS)/somref.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somref/$(PLATFORM)/$(BUILDTYPE)/bundle.somref.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somref/$(PLATFORM)/$(BUILDTYPE)/bundle.somref.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK

$(PLUGINS)/somcslib.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somcslib/$(PLATFORM)/$(BUILDTYPE)/bundle.somcslib.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somcslib/$(PLATFORM)/$(BUILDTYPE)/bundle.somcslib.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK

$(PLUGINS)/somir.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/bundle.somir.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somir/$(PLATFORM)/$(BUILDTYPE)/bundle.somir.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK 

$(PLUGINS)/somu.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somu/$(PLATFORM)/$(BUILDTYPE)/bundle.somu.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somu/$(PLATFORM)/$(BUILDTYPE)/bundle.somu.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(PLUGINS)/somu2.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somu2/$(PLATFORM)/$(BUILDTYPE)/bundle.somu2.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somu2/$(PLATFORM)/$(BUILDTYPE)/bundle.somu2.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK

$(PLUGINS)/somcorba.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somcorba/$(PLATFORM)/$(BUILDTYPE)/bundle.somcorba.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somcorba/$(PLATFORM)/$(BUILDTYPE)/bundle.somcorba.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK

$(PLUGINS)/somdcomm.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somdcomm/$(PLATFORM)/$(BUILDTYPE)/bundle.somdcomm.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somdcomm/$(PLATFORM)/$(BUILDTYPE)/bundle.somdcomm.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(PLUGINS)/somcdr.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somcdr/$(PLATFORM)/$(BUILDTYPE)/bundle.somcdr.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somcdr/$(PLATFORM)/$(BUILDTYPE)/bundle.somcdr.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK

$(PLUGINS)/somd.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/bundle.somd.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somd/$(PLATFORM)/$(BUILDTYPE)/bundle.somd.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(PLUGINS)/somabs1.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somabs1/$(PLATFORM)/$(BUILDTYPE)/bundle.somabs1.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somabs1/$(PLATFORM)/$(BUILDTYPE)/bundle.somabs1.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK

$(PLUGINS)/somestrm.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somestrm/$(PLATFORM)/$(BUILDTYPE)/bundle.somestrm.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somestrm/$(PLATFORM)/$(BUILDTYPE)/bundle.somestrm.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(PLUGINS)/somnmf.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somnmf/$(PLATFORM)/$(BUILDTYPE)/bundle.somnmf.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somnmf/$(PLATFORM)/$(BUILDTYPE)/bundle.somnmf.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(PLUGINS)/somos.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somos/$(PLATFORM)/$(BUILDTYPE)/bundle.somos.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somos/$(PLATFORM)/$(BUILDTYPE)/bundle.somos.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK $(UUIDLIBS)

$(PLUGINS)/somany.dll: SOMTK
	$(CC) $(CFLAGS) $(BUNDLE) -o $@ \
		$(BUILD)/somany/$(PLATFORM)/$(BUILDTYPE)/bundle.somany.SOMInitModule.c \
		-Wl,-exported_symbols_list,$(BUILD)/somany/$(PLATFORM)/$(BUILDTYPE)/bundle.somany.def \
		-F$(FRAMEWORKS) -F$(SUBFRAMEWORKS) -framework SOMTK

$(OBJDIR)/somkpath.o: $(SRCDIR)/somkpath.c $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/somkpath.c -o $@ -I$(BUILD)/som/include

$(BINDIR)/somipc: $(OUTDIR_BIN)/somipc
	cp $(OUTDIR_BIN)/somipc $@
