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

TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somst$(DLLSUFFIX)
OBJS=$(INTDIR)/tcpsock.o
INCL=	-I$(INTDIR) \
		-I.. \
		-I../../somidl/$(PLATFORM) \
		-I../../somkpub/include		\
		-I../../somtk/include		\
		$(STDINCL) -I../../soms/include

all: $(TARGET)

clean:
	$(CLEAN) $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(OBJS) \
		`$(SHLB_EXP) somst somst` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_REF) soms soms` \
		`$(SHLB_REF) somu somu` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) som som` \
		`$(SHLB_MAP) somst somst` \
		$(SOCKLIBS) \
		$(STDLIB) \
		$(LINKDLL_TAIL)

$(INTDIR)/tcpsock.o: ../src/tcpsock.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../src/tcpsock.c -o $@

dist install:

