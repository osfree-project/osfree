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

OBJS=$(INTDIR)/rhbmtut.o $(INTDIR)/rhbmtutx.o $(INTDIR)/rhbmtutp.o
TARGET=$(OUTDIR_SHLIB)/$(SHLIBPREFIX)rhbmtut$(SHLIBSUFFIX) 
INCLS=-I../include $(STDINCL)

all: $(TARGET)

clean:
	$(CLEAN) $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	if test -f "$@"; then rm "$@"; fi
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@\
		$(OBJS) \
		`$(SHLB_EXP) rhbmtut rhbmtut` \
		`$(SHLB_MAP) rhbmtut rhbmtut` \
		$(STDLIB) \
		$(LINKDLL_TAIL)

$(INTDIR)/rhbmtut.o: ../src/rhbmtut.c 
	$(CC_DLL) $(STDOPT) $(INCLS) -DBUILD_RHBMTUT -c ../src/rhbmtut.c -o $@

$(INTDIR)/rhbmtutx.o: ../src/rhbmtutx.c 
	$(CC_DLL) $(STDOPT) $(INCLS) -DBUILD_RHBMTUT -c ../src/rhbmtutx.c -o $@

$(INTDIR)/rhbmtutp.o: ../src/rhbmtutp.c 
	$(CC_DLL) $(STDOPT) $(INCLS) -DBUILD_RHBMTUT -c ../src/rhbmtutp.c -o $@

dist install:

