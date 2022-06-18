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

TARGET=$(OUTDIR_BIN)/regimpl$(EXESUFFIX)
INCL=		-I$(INTDIR) \
			-I../include \
			-I../../somidl/$(PLATFORM) \
			-I../../somcdr \
			-I../../somtc \
			-I../../somkpub/include		\
			-I../../somtk/include		\
			$(STDINCL)
OBJS=$(INTDIR)/regimpl.o $(INTDIR)/regior.o $(INTDIR)/regiortc.o

all: $(TARGET) $(OUTDIR_MAN)/regimpl.8

clean:
	$(CLEAN) $(OBJS) $(TARGET) $(INTDIR)/regiortc.tc $(OUTDIR_MAN)/regimpl.8 $(OUTDIR_MAN)/regimpl.8.*

$(INTDIR)/regimpl.o: ../src/regimpl.c
	$(CC_EXE) $(STDOPT) $(INCL)  -c ../src/regimpl.c -o $@

$(INTDIR)/regior.o: ../src/regior.c
	$(CC_EXE) $(STDOPT) $(INCL)  -c ../src/regior.c -o $@

$(INTDIR)/regiortc.o: ../src/regiortc.c $(INTDIR)/regiortc.tc
	$(CC_EXE) $(STDOPT) $(INCL)  -c ../src/regiortc.c -o $@

$(INTDIR)/regiortc.tc: ../regiortc.idl 
	$(SC) -p -stc -D __GENERATE_REGIMPL__ -I.. -I../../somidl -d $(INTDIR) ../regiortc.idl

$(TARGET): $(OBJS)
	$(LINKAPP) $(LINKAPP_HEAD) $(OBJS) -o $@ \
		`$(SHLB_REF) somd somd` \
		`$(SHLB_REF) somdcomm somdcomm` \
		`$(SHLB_REF) somem somem` \
		`$(SHLB_REF) somir somir` \
		`$(SHLB_REF) somu somu` \
		`$(SHLB_REF) somcdr somcdr` \
		`$(SHLB_REF) somestrm somestrm` \
		`$(SHLB_REF) somtc somtc` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) somos somos` \
		`$(SHLB_REF) som som` \
		$(UUIDLIBS) $(STDLIB) $(LINKAPP_TAIL)

dist install:

$(OUTDIR_MAN)/regimpl.8: ../unix/manpage.txt
	../../toolbox/txt2man.sh <../unix/manpage.txt >$@
	../../toolbox/manpack.sh $@
