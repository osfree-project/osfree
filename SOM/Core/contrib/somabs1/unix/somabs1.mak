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

TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somabs1$(DLLSUFFIX)
INCL=		-I$(INTDIR) \
			-I../include \
			-I../../somidl/$(PLATFORM) \
			-I../../somkpub/include		\
			-I../../somtk/include		\
			$(STDINCL)
OBJS=$(INTDIR)/omgidobj.o $(INTDIR)/somabs1.o

all: $(TARGET)

clean:
	$(CLEAN) $(OBJS) $(TARGET)

$(TARGET): $(OBJS) 
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(OBJS) \
		$(STDLIB) \
		`$(SHLB_EXP) somabs1 somabs1` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) som som` \
		`$(SHLB_MAP) somabs1 somabs1` \
		$(LINKDLL_TAIL)

$(INTDIR)/somabs1.o: ../src/somabs1.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/somabs1.c -o $@

$(INTDIR)/omgidobj.o: ../src/omgidobj.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/omgidobj.c -o $@

dist install:

