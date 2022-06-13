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

TARGET=$(OUTDIR_TESTS)/ipv6test$(EXESUFFIX)
OBJS=$(INTDIR)/ipv6test.o
IPV6TEST_H=$(INTDIR)/ipv6test.h

all: $(TARGET) $(IPV6TEST_H)

clean:
	$(CLEAN) $(TARGET) $(OBJS) $(IPV6TEST_H)

$(INTDIR)/ipv6test.o: ../src/ipv6test.c
	$(CC_EXE) $(STDOPT) $(STDINCL) -c ../src/ipv6test.c -o $@

$(TARGET): $(OBJS)
	$(LINKAPP) $(LINKAPP_HEAD) $(OBJS) \
		-o $@ \
		$(SOCKLIBS) \
		$(STDLIB) \
		$(LINKAPP_TAIL)

$(IPV6TEST_H): $(TARGET)
	$(EXEC_CONFIG) $(TARGET) $@
	if test ! -f $@; then exit 1; fi


dist install:

