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

TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somd$(DLLSUFFIX)

OBJS=	$(INTDIR)/rhbsomd.o \
		$(INTDIR)/somoa.o \
		$(INTDIR)/orb.o \
		$(INTDIR)/somdobj.o \
		$(INTDIR)/somdcprx.o \
		$(INTDIR)/boa.o \
		$(INTDIR)/request.o \
		$(INTDIR)/somdserv.o \
		$(INTDIR)/servmgr.o \
		$(INTDIR)/rhbgiop2.o \
		$(INTDIR)/rhbgiop3.o \
		$(INTDIR)/rhbgiop4.o \
		$(INTDIR)/rhbgiop5.o \
		$(INTDIR)/rhbgiop6.o \
		$(INTDIR)/rhbgiop7.o \
		$(INTDIR)/rhbgiop8.o \
		$(INTDIR)/rhbgiop9.o \
		$(INTDIR)/somdom.o \
		$(INTDIR)/nvlist.o \
		$(INTDIR)/impldef.o \
		$(INTDIR)/implrep.o \
		$(INTDIR)/rhbgiop1.o \
		$(INTDIR)/principl.o \
		$(INTDIR)/cntxt.o \
		$(INTDIR)/om.o \
		$(INTDIR)/rhbgiops.o \
		$(INTDIR)/somdmprx.o \
		$(INTDIR)/poa.o \
		$(INTDIR)/rhborbst.o \
		$(INTDIR)/unotypes.o \
		$(INTDIR)/somdtcdt.o \
		$(INTDIR)/servreq.o \
		$(INTDIR)/somdfenc.o \
		$(INTDIR)/somdstub.o # $(INTDIR)/rhbsomkd.o $(INTDIR)/rhbsomup.o 

SOMDCPRX_H=$(INTDIR)/somdcprx.h
SOMDCPRX_IH=$(INTDIR)/somdcprx.ih
SOMDOBJ_H=$(INTDIR)/somdobj.h
SOMDOBJ_IH=$(INTDIR)/somdobj.ih
RHBTYPES_H=$(INTDIR)/rhbtypes.h

SOMDINCL=	-I.. 					\
			-I../../somkpub/som 	\
			-I../../somidl			

SOMDIDL=$(SOMDCPRX_H) \
		$(SOMDCPRX_IH) \
		$(SOMDOBJ_H) \
		$(SOMDOBJ_IH) \
		$(RHBTYPES_H) 

all: $(SOMDIDL) $(TARGET)

clean:
	$(CLEAN) $(OBJS) $(TARGET) $(INTDIR)/somdtcdt.tc $(SOMDIDL)

SOMD_INCL=	-I$(INTDIR)									\
			-I../include								\
			-I../../somcdr/include						\
			-I../../somu/include						\
			-I../../somtc/include						\
			-I../../som/$(PLATFORM)/$(INTDIR)			\
			-I../../somref/$(PLATFORM)/$(INTDIR)		\
			-I../../somidl/$(PLATFORM)					\
			-I../../somcdr/$(PLATFORM)/$(INTDIR)		\
			-I../../soms/include						\
			-I../../ipv6test/$(PLATFORM)/$(BUILDTYPE) 	\
			-I../../rhbmtut/include						\
			-I../../somkpub/include						\
			-I../../somtk/include						\
			-DBUILD_SOMD								\
			$(STDINCL)

$(TARGET): $(OBJS)
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(OBJS) \
		`$(SHLB_EXP) somd somd` \
		`$(SHLB_REF) som som` \
		`$(SHLB_REF) somtc somtc` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) somcorba somcorba` \
		`$(SHLB_REF) somany somany` \
		`$(SHLB_REF) somcdr somcdr` \
		`$(SHLB_REF) somu somu` \
		`$(SHLB_REF) somestrm somestrm` \
		`$(SHLB_REF) somabs1 somabs1` \
		`$(SHLB_REF) somos somos` \
		`$(SHLB_REF) somem somem` \
		`$(SHLB_REF) somst somst` \
		`$(SHLB_REF) soms soms` \
		`$(SHLB_REF) somnmf somnmf` \
		`$(SHLB_REF) somdcomm somdcomm` \
		`$(SHLB_REF) uuid uuid` \
		`$(ARLB_REF) rhbmtuta rhbmtuta` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_MAP) somd somd` \
		$(SOCKLIBS) $(UUIDLIBS) $(STDLIB)  \
		$(LINKDLL_TAIL)

$(INTDIR)/rhbsomd.o: ../src/rhbsomd.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/rhbsomd.c -o $@

$(INTDIR)/orb.o: ../src/orb.c 
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/orb.c -o $@

$(INTDIR)/somoa.o: ../src/somoa.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/somoa.c -o $@

$(INTDIR)/boa.o: ../src/boa.c 
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/boa.c -o $@

$(INTDIR)/somdobj.o: ../src/somdobj.c 
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/somdobj.c -o $@

$(INTDIR)/somdcprx.o: ../src/somdcprx.c 
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/somdcprx.c -o $@

$(INTDIR)/request.o: ../src/request.c 
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/request.c -o $@

$(INTDIR)/somdserv.o: ../src/somdserv.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/somdserv.c -o $@

$(INTDIR)/servmgr.o: ../src/servmgr.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/servmgr.c -o $@

$(INTDIR)/rhbgiop1.o: ../src/rhbgiop1.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/rhbgiop1.c -o $@

$(INTDIR)/rhbgiop2.o: ../src/rhbgiop2.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/rhbgiop2.c -o $@

$(INTDIR)/rhbgiop3.o: ../src/rhbgiop3.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/rhbgiop3.c -o $@

$(INTDIR)/rhbgiop4.o: ../src/rhbgiop4.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/rhbgiop4.c -o $@

$(INTDIR)/rhbgiop5.o: ../src/rhbgiop5.c 
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/rhbgiop5.c -o $@

$(INTDIR)/rhbgiop6.o: ../src/rhbgiop6.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL)  -c ../src/rhbgiop6.c -o $@

$(INTDIR)/rhbgiop7.o: ../src/rhbgiop7.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL)  -c ../src/rhbgiop7.c -o $@

$(INTDIR)/rhbgiop8.o: ../src/rhbgiop8.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL)  -c ../src/rhbgiop8.c -o $@

$(INTDIR)/rhbgiop9.o: ../src/rhbgiop9.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL)  -c ../src/rhbgiop9.c -o $@

$(INTDIR)/rhbgiops.o: ../src/rhbgiops.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/rhbgiops.c -o $@

$(INTDIR)/rhbgiope.o: ../src/rhbgiope.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -I../../uuid -c ../src/rhbgiope.c -o $@

$(INTDIR)/somdom.o: ../src/somdom.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/somdom.c -o $@

$(INTDIR)/impldef.o: ../src/impldef.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -I../../uuid  -c ../src/impldef.c -o $@

$(INTDIR)/implrep.o: ../src/implrep.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/implrep.c -o $@

$(INTDIR)/nvlist.o: ../src/nvlist.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/nvlist.c -o $@

$(INTDIR)/cntxt.o: ../src/cntxt.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/cntxt.c -o $@

$(INTDIR)/principl.o: ../src/principl.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/principl.c -o $@

$(INTDIR)/somdmprx.o: ../src/somdmprx.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/somdmprx.c -o $@

$(INTDIR)/om.o: ../src/om.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/om.c -o $@

$(INTDIR)/rhbsomkd.o: ../../somcdr/src/rhbsomkd.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../../somcdr/src/rhbsomkd.c -o $@

$(INTDIR)/rhbsomup.o: ../../somcdr/src/rhbsomup.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../../somcdr/src/rhbsomup.c -o $@

$(INTDIR)/poa.o: ../src/poa.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/poa.c -o $@

$(INTDIR)/servreq.o: ../src/servreq.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/servreq.c -o $@

$(INTDIR)/somdstub.o: ../src/somdstub.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/somdstub.c -o $@

$(INTDIR)/rhborbst.o: ../src/rhborbst.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/rhborbst.c -o $@

$(INTDIR)/unotypes.o: ../src/unotypes.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/unotypes.c -o $@

$(INTDIR)/somdtcdt.o: ../src/somdtcdt.c $(INTDIR)/somdtcdt.tc
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL) -c ../src/somdtcdt.c -o $@

$(INTDIR)/somdtcdt.tc: ../somdtcdt.idl $(SC)
	$(SC) -p -stc -D __GENERATE_SOMD__ -I.. -I../../somidl -d $(INTDIR) ../somdtcdt.idl

$(INTDIR)/somdfenc.o: ../src/somdfenc.c
	$(CC_DLL) $(STDOPT)  $(SOMD_INCL)  -c ../src/somdfenc.c -o $@

$(SOMDCPRX_H): ../../somd/somdcprx.idl ../../somd/somdobj.idl ../../somkpub/som/somobj.idl
	$(SC) -s h -p $(SOMDINCL) ../somdcprx.idl -d $(INTDIR)

$(SOMDCPRX_IH): ../../somd/somdcprx.idl ../../somd/somdobj.idl ../../somkpub/som/somobj.idl
	$(SC) -s ih -p $(SOMDINCL) ../somdcprx.idl -d $(INTDIR)

$(SOMDOBJ_H): ../somdobj.idl ../../somkpub/som/somobj.idl
	$(SC) -s h -p $(SOMDINCL) ../somdobj.idl -d $(INTDIR)

$(SOMDOBJ_IH): ../../somd/somdobj.idl ../../somkpub/som/somobj.idl
	$(SC) -s ih -p $(SOMDINCL) ../somdobj.idl -d $(INTDIR)

$(RHBTYPES_H): ../rhbtypes.idl
	$(SC) -s h -p $(SOMDINCL) ../rhbtypes.idl -d $(INTDIR)

dist install:

