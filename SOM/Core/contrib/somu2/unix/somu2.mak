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

TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somu2$(DLLSUFFIX)
OBJS=$(INTDIR)/rhbsomu2.o $(INTDIR)/somtdm.o
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
		$(OBJS) $(STDLIB) \
		`$(SHLB_EXP) somu2 somu2` \
		`$(SHLB_REF) som som` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_MAP) somu2 somu2` \
		$(LINKDLL_TAIL)

$(INTDIR)/rhbsomu2.o: ../src/rhbsomu2.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/rhbsomu2.c -o $@

$(INTDIR)/somtdm.o: ../src/somtdm.c
	$(CC_DLL) $(STDOPT) $(INCL) -c ../src/somtdm.c -o $@


dist install:

