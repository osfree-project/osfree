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

TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somref$(DLLSUFFIX)
OBJS=$(INTDIR)/somref.o
INCL=	-I$(INTDIR) \
		-I. \
		-I.. \
		-I../../somidl/$(PLATFORM) \
		-I../../somkpub/include \
		-I../../somtk/include \
		$(STDINCL)

CC_OPT=$(STDOPT) $(INCL) 

all: $(TARGET)

clean:
	$(CLEAN) $(OBJS) $(TARGET)

$(TARGET): $(OBJS)
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ $(OBJS) \
		$(STDLIB) \
		`$(SHLB_EXP) somref somref` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_REF) som som` \
		`$(SHLB_MAP) somref somref` \
		$(LINKDLL_TAIL)

$(INTDIR)/somref.o: ../src/somref.c
	$(CC_DLL) $(CC_OPT) -c ../src/somref.c -o $@

dist install:

