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

TARGETREFLIB=$(OUTDIR_REFLIB)
TARGETIMPLIB=$(OUTDIR_IMPLIB)

TARGETS= \
	$(TARGETREFLIB)/$(DLLPREFIX)somir$(DLLSUFFIX)

INCL=	-I../../somkpub/include \
		-I../../somtk/include \
		-I../../somidl/$(PLATFORM) \
		-I../../somstubs/include \
		$(STDINCL) \
		-DBUILD_SOMSTUBS

OBJS=$(INTDIR)/somir.o 

all: $(TARGETS)

clean:
	$(CLEAN) $(OBJS) $(TARGETS)

$(TARGETREFLIB)/$(DLLPREFIX)somir$(DLLSUFFIX): $(INTDIR)/somir.o ../../somir/unix/somir.exp
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(INTDIR)/somir.o \
		$(STDLIB) \
		`$(SHLB_EXP) somir somir` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_MAP) somir somir` \
		$(LINKDLL_TAIL)

$(INTDIR)/somir.o: ../../somir/src/stub.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../../somir/src/stub.c -o $@


dist install:

