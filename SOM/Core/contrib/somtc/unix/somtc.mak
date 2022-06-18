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

TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somtc$(DLLSUFFIX)
OBJS=$(INTDIR)/rhbsomtc.o $(INTDIR)/rhbsomva.o $(INTDIR)/rhbsomts.o
INCL=	-I$(INTDIR) \
		-I../include \
		-I../../somidl/$(PLATFORM) \
		-I../../somkpub/include \
		-I../../somtk/include \
		$(STDINCL) \
		-DBUILD_SOMTC

all: $(TARGET)

clean:
	$(CLEAN) $(OBJS) $(TARGET) $(INTDIR)/somtcdat.tc

$(TARGET): $(OBJS)
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ $(OBJS) \
		`$(SHLB_EXP) somtc somtc` \
		`$(SHLB_REF) som som` \
		`$(SHLB_MAP) somtc somtc` \
		$(STDLIB) \
		$(LINKDLL_TAIL)

$(INTDIR)/rhbsomtc.o: ../src/rhbsomtc.c $(INTDIR)/somtcdat.tc
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/rhbsomtc.c -o $@

$(INTDIR)/rhbsomva.o: ../src/rhbsomva.c ../include/rhbvargs.h
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/rhbsomva.c -o $@

$(INTDIR)/rhbsomts.o: ../src/rhbsomts.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/rhbsomts.c -o $@

$(INTDIR)/somtcdat.tc: ../somtcdat.idl
	$(SC) -stc -D __GENERATE_SOMTC__ -I../../somidl -d $(INTDIR) ../somtcdat.idl

dist install:

