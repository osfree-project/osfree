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

OBJ=$(INTDIR)/rhbmtut.o $(INTDIR)/rhbmtutx.o $(INTDIR)/rhbmtutp.o
TMPLIB=$(INTDIR)/librhbmtuta.a
TARGET=$(OUTDIR_LIB)/librhbmtuta.a
PART_OPTS=-I../../rhbmtut/include $(STDOPT) $(STDINCL) -DBUILD_RHBMTUT

all: $(TARGET)

clean:
	$(CLEAN) $(TARGET) $(OBJ) $(TMPLIB)

$(INTDIR)/rhbmtut.o: ../../rhbmtut/src/rhbmtut.c 
	$(CC_DLL) $(PART_OPTS) -c ../../rhbmtut/src/rhbmtut.c -o $@

$(INTDIR)/rhbmtutx.o: ../../rhbmtut/src/rhbmtutx.c 
	$(CC_DLL) $(PART_OPTS) -c ../../rhbmtut/src/rhbmtutx.c -o $@

$(INTDIR)/rhbmtutp.o: ../../rhbmtut/src/rhbmtutp.c 
	$(CC_DLL) $(PART_OPTS) -c ../../rhbmtut/src/rhbmtutp.c -o $@

$(TMPLIB): $(OBJ)
	cd `dirname $@`; for d in $(OBJ); \
	do if test -f `basename $$d`; \
		then $(AR) $(ARFLAGS) $(PLATFORM_ARFLAGS) `basename $@` `basename $$d`; \
	fi ; \
	done
	if test -f $@; \
	then if test "$(RANLIB)" != ""; \
		then cd `dirname $@`; $(RANLIB) `basename $@`; \
		fi; \
	fi

$(TARGET): $(TMPLIB)
	cp $(TMPLIB) $@


dist install:

