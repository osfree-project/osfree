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

OBJS=$(INTDIR)/somos.o \
	$(INTDIR)/rhbsomos.o \
	$(INTDIR)/somap.o \
	$(INTDIR)/somosutl.o # $(INTDIR)/rhbsomup.o 
INCL=	-I$(INTDIR) \
		-I../include \
		-I../../somidl/$(PLATFORM) \
		-I../../somkpub/include		\
		-I../../somtk/include		\
		$(STDINCL)
TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somos$(DLLSUFFIX)

all: $(TARGET)

clean:
	$(CLEAN) $(OBJS) $(TARGET)

$(TARGET): $(OBJS) 
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(OBJS) \
		`$(SHLB_EXP) somos somos` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_REF) somd somd` \
		`$(SHLB_REF) somcdr somcdr` \
		`$(SHLB_REF) somu somu` \
		`$(SHLB_REF) somu2 somu2` \
		`$(SHLB_REF) somabs1 somabs1` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) somtc somtc` \
		`$(SHLB_REF) som som` \
		`$(SHLB_MAP) somos somos` \
		$(STDLIB) \
		$(LINKDLL_TAIL)

$(INTDIR)/somos.o: ../src/somos.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/somos.c -o $@

$(INTDIR)/somosutl.o: ../src/somosutl.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/somosutl.c -o $@

$(INTDIR)/rhbsomos.o: ../src/rhbsomos.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/rhbsomos.c -o $@

$(INTDIR)/rhbsomup.o: ../../somcdr/src/rhbsomup.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../../somcdr/src/rhbsomup.c -o $@

$(INTDIR)/somap.o: ../src/somap.c 
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/somap.c -o $@

dist install:

