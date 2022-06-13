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

OBJS=	$(INTDIR)/defedidl.o \
		$(INTDIR)/eda.o \
		$(INTDIR)/fma.o \
		$(INTDIR)/fmi.o \
		$(INTDIR)/fmib.o \
		$(INTDIR)/fsagm.o \
		$(INTDIR)/fsgm.o \
		$(INTDIR)/iogma.o \
		$(INTDIR)/iogrp.o \
		$(INTDIR)/mia.o \
		$(INTDIR)/nsma.o \
		$(INTDIR)/objset.o \
		$(INTDIR)/pid.o \
		$(INTDIR)/po.o \
		$(INTDIR)/poid.o \
		$(INTDIR)/poida.o \
		$(INTDIR)/psma.o \
		$(INTDIR)/rhbsomp.o \
		$(INTDIR)/srkset.o
TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somp$(DLLSUFFIX)
INCL=	-I$(INTDIR) \
		-I../include \
		-I../../somidl/$(PLATFORM) \
		-I../../somkpub/include		\
		-I../../somtk/include		\
		$(STDINCL)

all: $(TARGET)

clean:
	$(CLEAN) $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(OBJS) \
		`$(SHLB_EXP) somp somp` \
		`$(SHLB_REF) somu somu` \
		`$(SHLB_REF) somcdr somcdr` \
		`$(SHLB_REF) somos somos` \
		`$(SHLB_REF) somabs1 somabs1` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) somtc somtc` \
		`$(SHLB_REF) som som` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_MAP) somp somp` \
		$(STDLIB) \
		$(LINKDLL_TAIL)

$(INTDIR)/rhbsomp.o: ../src/rhbsomp.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/rhbsomp.c -o $@

$(INTDIR)/po.o: ../src/po.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/po.c -o $@

$(INTDIR)/defedidl.o: ../src/defedidl.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/defedidl.c -o $@

$(INTDIR)/eda.o: ../src/eda.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/eda.c -o $@

$(INTDIR)/fma.o: ../src/fma.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/fma.c -o $@

$(INTDIR)/fmi.o: ../src/fmi.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/fmi.c -o $@

$(INTDIR)/fmib.o: ../src/fmib.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/fmib.c -o $@

$(INTDIR)/fsagm.o: ../src/fsagm.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/fsagm.c -o $@

$(INTDIR)/fsgm.o: ../src/fsgm.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/fsgm.c -o $@

$(INTDIR)/iogma.o: ../src/iogma.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/iogma.c -o $@

$(INTDIR)/iogrp.o: ../src/iogrp.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/iogrp.c -o $@

$(INTDIR)/mia.o: ../src/mia.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/mia.c -o $@

$(INTDIR)/nsma.o: ../src/nsma.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/nsma.c -o $@

$(INTDIR)/objset.o: ../src/objset.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/objset.c -o $@

$(INTDIR)/pid.o: ../src/pid.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/pid.c -o $@

$(INTDIR)/poid.o: ../src/poid.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/poid.c -o $@

$(INTDIR)/poida.o: ../src/poida.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/poida.c -o $@

$(INTDIR)/psma.o: ../src/psma.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/psma.c -o $@

$(INTDIR)/srkset.o: ../src/srkset.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/srkset.c -o $@


dist install:

