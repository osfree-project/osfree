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

TARGET=$(OUTDIR_BIN)/irdump$(EXESUFFIX)
OBJS=$(INTDIR)/irdump.o
INCL=	-I$(INTDIR) \
		-I.. \
		-I../../somidl/$(PLATFORM) \
		-I../../somkpub/include \
		-I../../somtk/include \
		$(STDINCL)

all: $(TARGET) $(OUTDIR_MAN)/irdump.1

clean:
	$(CLEAN) $(OBJS) $(TARGET) $(OUTDIR_MAN)/irdump.1 $(OUTDIR_MAN)/irdump.1.*

$(INTDIR)/irdump.o: ../src/irdump.c
	$(CC_EXE) $(STDOPT) $(INCL) -c ../src/irdump.c -o $@

$(TARGET): $(OBJS)
	$(LINKAPP) $(LINKAPP_HEAD) $(OBJS) -o $@ \
		`$(SHLB_REF) somir somir` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) som som` \
		$(STDLIB) \
		$(LINKAPP_TAIL)

test:
	SOMIR=$(SOMIR) $(EXEC_TEST) $(TARGET) \
		::SOMObject::somGetClass \
		::SOMClass::somNew \
		::NamedValue \
		::Identifier \
		::somModifier

dist install:

$(OUTDIR_MAN)/irdump.1: ../unix/manpage.txt
	../../toolbox/txt2man.sh <../unix/manpage.txt >$@
	../../toolbox/manpack.sh $@
