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

OBJS=$(INTDIR)/formarsh.o $(INTDIR)/somdcomm.o $(INTDIR)/rhbsomus.o
TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somdcomm$(DLLSUFFIX)
INCL=-I$(INTDIR) \
	-I.. \
	-I../../somidl/$(PLATFORM) \
	-I../../somcdr/include \
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
		`$(SHLB_EXP) somdcomm somdcomm` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_REF) somestrm somestrm` \
		`$(SHLB_REF) somcorba somcorba` \
		`$(SHLB_REF) somcdr somcdr` \
		`$(SHLB_REF) somabs1 somabs1` \
		`$(SHLB_REF) somos somos` \
		`$(SHLB_REF) somtc somtc` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) som som` \
		`$(SHLB_MAP) somdcomm somdcomm` \
		$(STDLIB) \
		$(LINKDLL_TAIL)

$(INTDIR)/formarsh.o: ../src/formarsh.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/formarsh.c -o $@ 

$(INTDIR)/somdcomm.o: ../src/somdcomm.c 
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/somdcomm.c -o $@ 

$(INTDIR)/rhbsomus.o: ../src/rhbsomus.c 
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/rhbsomus.c -o $@ 

dist install:

