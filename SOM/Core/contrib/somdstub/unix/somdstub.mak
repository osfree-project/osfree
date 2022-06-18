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
	$(TARGETIMPLIB)/$(DLLPREFIX)somos$(DLLSUFFIX) \
	$(TARGETIMPLIB)/$(DLLPREFIX)somnmf$(DLLSUFFIX) \
	$(TARGETIMPLIB)/$(DLLPREFIX)somestrm$(DLLSUFFIX)

INCL=		-I../include \
			-I../../somidl/$(PLATFORM) \
			-I../../somstubs/include	\
			-I../../somkpub/include		\
			-I../../somtk/include		\
			 $(STDINCL) \
			-DBUILD_SOMSTUBS
OBJS=$(INTDIR)/somos.o $(INTDIR)/somnmf.o $(INTDIR)/somestrm.o 

all: $(TARGETS)

clean:
	$(CLEAN) $(OBJS) $(TARGETS)

$(TARGETIMPLIB)/$(DLLPREFIX)somos$(DLLSUFFIX): $(INTDIR)/somos.o ../../somos/unix/somos.exp
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(INTDIR)/somos.o \
		$(STDLIB) \
		`$(SHLB_EXP) somos somos` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_MAP) somos somos` \
		$(LINKDLL_TAIL)

$(TARGETIMPLIB)/$(DLLPREFIX)somnmf$(DLLSUFFIX): $(INTDIR)/somnmf.o ../../somnmf/unix/somnmf.exp
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(INTDIR)/somnmf.o \
		$(STDLIB) \
		`$(SHLB_EXP) somnmf somnmf` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_MAP) somnmf somnmf` \
		$(LINKDLL_TAIL)

$(TARGETIMPLIB)/$(DLLPREFIX)somestrm$(DLLSUFFIX): $(INTDIR)/somestrm.o ../../somestrm/unix/somestrm.exp
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(INTDIR)/somestrm.o \
		$(STDLIB) \
		`$(SHLB_EXP) somestrm somestrm` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_MAP) somestrm somestrm` \
		$(LINKDLL_TAIL)

$(INTDIR)/somnmf.o: ../../somnmf/src/stub.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../../somnmf/src/stub.c -o $@

$(INTDIR)/somos.o: ../../somos/src/stub.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../../somos/src/stub.c -o $@

$(INTDIR)/somestrm.o: ../../somestrm/src/stub.c
	$(CC_DLL) $(STDOPT) $(INCL)  -c ../../somestrm/src/stub.c -o $@



dist install:

