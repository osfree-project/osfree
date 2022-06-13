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

PART=somcdr

TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)$(PART)$(DLLSUFFIX)

OBJS=	$(INTDIR)/rhbsomut.o \
		$(INTDIR)/rhbsomkd.o \
		$(INTDIR)/rhbsomue.o \
		$(INTDIR)/somcdr.o 

INCL=	-I$(INTDIR) 	\
		-I../include 	\
		-I../../somidl/$(PLATFORM) \
		-I../../rhbmtut/include \
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
		`$(SHLB_EXP) $(PART) $(PART)` \
		`$(SHLB_REF) somcorba somcorba` \
		`$(SHLB_REF) somtc somtc` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) som som` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_MAP) $(PART) $(PART)` \
		$(STDLIB) \
		$(LINKDLL_TAIL)

$(INTDIR)/rhbsomut.o: ../src/rhbsomut.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMCDR  -c ../src/rhbsomut.c -o $@

$(INTDIR)/rhbsomkd.o: ../src/rhbsomkd.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMCDR  -c ../src/rhbsomkd.c -o $@

$(INTDIR)/rhbsomue.o: ../src/rhbsomue.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMCDR  -c ../src/rhbsomue.c -o $@

$(INTDIR)/somcdr.o: ../src/somcdr.c
	$(CC_DLL) $(STDOPT) $(INCL) -DBUILD_SOMCDR  -c ../src/somcdr.c -o $@


dist install:

