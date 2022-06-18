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

OBJS=$(INTDIR)/rhbseh.o
TARGET=$(OUTDIR_SHLIB)/$(SHLIBPREFIX)rhbseh$(SHLIBSUFFIX) 

all: $(TARGET)

clean:
	$(CLEAN) $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	if test -f "$@"; then rm "$@"; fi
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@\
		$(OBJS) \
		`$(SHLB_ENT) rhbseh_init` \
		`$(SHLB_EXP) rhbseh rhbseh` \
		`$(SHLB_MAP) rhbseh rhbseh` \
		$(STDLIB) \
		$(LINKDLL_TAIL)

$(INTDIR)/rhbseh.o: ../src/rhbseh.c 
	$(CC_DLL) $(STDOPT) $(STDINCL) -I../include $(RHBSEH_CFLAGS) -DBUILD_RHBSEH -c ../src/rhbseh.c -o $@

dist install:

