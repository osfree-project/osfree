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

OBJS=	$(INTDIR)/biter.o \
		$(INTDIR)/lname.o \
		$(INTDIR)/lnamec.o \
		$(INTDIR)/naming.o \
		$(INTDIR)/rhbsomnm.o \
		$(INTDIR)/xnaming.o \
		$(INTDIR)/xnamingf.o

SOMNMF_INCL=-I$(INTDIR) 	\
			-I../include	\
			-I../../somcdr/$(PLATFORM)/$(INTDIR) \
			-I../../somcdr/include	\
			-I../../somidl/$(PLATFORM) \
			-I../../rhbmtut/include \
			-I../../somkpub/include		\
			-I../../somtk/include		\
			$(STDINCL)

TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somnmf$(DLLSUFFIX)

all: $(TARGET)

clean:
	$(CLEAN) $(OBJS) $(TARGET)

$(TARGET): $(OBJS) 
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(OBJS)  \
		`$(SHLB_EXP) somnmf somnmf` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_REF) somos somos` \
		`$(SHLB_REF) somcdr somcdr` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) somtc somtc` \
		`$(SHLB_REF) som som` \
		`$(SHLB_MAP) somnmf somnmf` \
		$(STDLIB) \
		$(LINKDLL_TAIL)

$(INTDIR)/biter.o: ../src/biter.c 
	$(CC_DLL) $(STDOPT) $(SOMNMF_INCL) -c ../src/biter.c -o $@

$(INTDIR)/lname.o: ../src/lname.c 
	$(CC_DLL) $(STDOPT) $(SOMNMF_INCL) -c ../src/lname.c -o $@

$(INTDIR)/lnamec.o: ../src/lnamec.c 
	$(CC_DLL) $(STDOPT) $(SOMNMF_INCL) -c ../src/lnamec.c -o $@

$(INTDIR)/naming.o: ../src/naming.c 
	$(CC_DLL) $(STDOPT) $(SOMNMF_INCL) -c ../src/naming.c -o $@

$(INTDIR)/rhbsomnm.o: ../src/rhbsomnm.c
	$(CC_DLL) $(STDOPT) $(SOMNMF_INCL) -c ../src/rhbsomnm.c -o $@

$(INTDIR)/xnaming.o: ../src/xnaming.c
	$(CC_DLL) $(STDOPT) $(SOMNMF_INCL)  -c ../src/xnaming.c -o $@

$(INTDIR)/xnamingf.o: ../src/xnamingf.c
	$(CC_DLL) $(STDOPT) $(SOMNMF_INCL)  -c ../src/xnamingf.c -o $@

dist install:

