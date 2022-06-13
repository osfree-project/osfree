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

TARGET=$(OUTDIR_BIN)/$(EXEPREFIX)somdsvr$(EXESUFFIX)
TARGET_C=$(OUTDIR_BIN)/$(EXEPREFIX)somdsvr-c$(EXESUFFIX)
TARGET_CPP=$(OUTDIR_BIN)/$(EXEPREFIX)somdsvr-cpp$(EXESUFFIX)
INCL=		-I. \
			-I.. \
			-I../../somidl/$(PLATFORM) \
			-I../../somkpub/include		\
			-I../../somtk/include		\
			$(STDINCL)
OBJS_CPP=$(INTDIR)/somdsvr-cpp.o
OBJS_C=$(INTDIR)/somdsvr-c.o

all: $(TARGET_C) $(TARGET_CPP) $(TARGET)

clean:
	$(CLEAN) $(TARGET) $(TARGET_C) $(TARGET_CPP) $(OBJS_C) $(OBJS_CPP)

$(INTDIR)/somdsvr-cpp.o: ../src/somdsvr.cpp ../src/somdsvr.c
	$(CXX_EXE) -I../src $(STDOPTXX) $(INCL)  -c ../src/somdsvr.cpp -o $@

$(INTDIR)/somdsvr-c.o: ../src/somdsvr.c
	$(CC_EXE) $(STDOPT) $(INCL)  -c ../src/somdsvr.c -o $@

$(TARGET_CPP): $(OBJS_CPP)
	$(LINKAPPXX) $(LINKAPP_HEAD) $(OBJS_CPP) -o $@ \
		`$(SHLB_REF) somd somd` \
		`$(SHLB_REF) somem somem` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) som som` \
		`$(SHLB_REF) somos somos` \
		`$(SHLB_MAP) somdsvr somdsvr-cpp` \
		$(UUIDLIBS) $(STDLIBXX) $(LINKAPP_TAIL)

$(TARGET_C): $(OBJS_C)
	$(LINKAPP) $(LINKAPP_HEAD) $(OBJS_C) -o $@ \
		`$(SHLB_REF) somd somd` \
		`$(SHLB_REF) somem somem` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) som som` \
		`$(SHLB_REF) somos somos` \
		`$(SHLB_MAP) somdsvr somdsvr-c` \
		$(UUIDLIBS) $(STDLIB) $(LINKAPP_TAIL)

$(TARGET): 
	cd `dirname $@`; ln -s `basename $(TARGET_C)` `basename $@`

dist install:

