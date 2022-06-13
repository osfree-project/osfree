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

TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somu$(DLLSUFFIX)

OBJS=$(INTDIR)/somproxy.o \
	 $(INTDIR)/snglicls.o \
	 $(INTDIR)/rhbsomu.o \
	 $(INTDIR)/rhbiniut.o \
	 $(INTDIR)/somsid.o \
	 $(INTDIR)/somida.o \
 	 $(INTDIR)/somthrd.o \
	 $(INTDIR)/sombacls.o
INCL=		-I$(INTDIR) 				\
			-I../src		 			\
			-I../include	 			\
			-I../../uuid/include		\
			-I../../somidl/$(PLATFORM) 	\
			-I../../somkpub/include		\
			-I../../somtk/include		\
			-I../../rhbiniut/include		\
			$(STDINCL)

all: $(TARGET)

clean:
	$(CLEAN) $(OBJS) $(TARGET)

$(TARGET): $(OBJS) 
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(OBJS) $(STDLIB) \
		`$(SHLB_EXP) somu somu` \
		`$(SHLB_REF) som som` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) uuid uuid` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_MAP) somu somu` \
		$(UUIDLIBS) $(LINKDLL_TAIL)

$(INTDIR)/snglicls.o: ../src/snglicls.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMU -c ../src/snglicls.c -o $@

$(INTDIR)/somproxy.o: ../src/somproxy.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMU -c ../src/somproxy.c -o $@

$(INTDIR)/rhbsomu.o: ../src/rhbsomu.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMU -c ../src/rhbsomu.c -o $@

$(INTDIR)/somsid.o: ../src/somsid.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMU -c ../src/somsid.c -o $@

$(INTDIR)/somida.o: ../src/somida.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMU -c ../src/somida.c -o $@

$(INTDIR)/somthrd.o: ../src/somthrd.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMU -c ../src/somthrd.c -o $@

$(INTDIR)/sombacls.o: ../src/sombacls.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMU -c ../src/sombacls.c -o $@

$(INTDIR)/rhbiniut.o: ../../rhbiniut/src/rhbiniut.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMU -c ../../rhbiniut/src/rhbiniut.c -o $@

dist install:

