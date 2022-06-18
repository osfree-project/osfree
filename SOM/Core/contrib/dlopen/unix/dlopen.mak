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

TARGET=$(OUTDIR_TOOLS)/dlopen$(EXESUFFIX)
OBJS=$(INTDIR)/dlopen.cpp.o
DLOPEN_C=../src/dlopen.c
DLOPEN_CPP=$(INTDIR)/dlopen.cpp

all: $(TARGET)

$(DLOPEN_CPP): $(DLOPEN_C)
	cp $(DLOPEN_C) $@

$(INTDIR)/dlopen.c.o: $(DLOPEN_C)
	$(CC_EXE) $(STDOPT) $(STDINCL) -c $(DLOPEN_C) -o $@

$(INTDIR)/dlopen.cpp.o: $(DLOPEN_CPP)
	$(CXX_EXE) $(STDOPTXX) $(STDINCL) -c $(DLOPEN_CPP) -o $@

$(TARGET): $(OBJS)
	$(LINKAPPXX) $(LINKAPP_HEAD) $(OBJS) \
		-o $@ \
		$(STDLIBXX) $(X11APP) \
		$(LINKAPP_TAIL)

clean:
	$(CLEAN) $(OBJS) $(TARGET) $(DLOPEN_CPP)

dist install:

