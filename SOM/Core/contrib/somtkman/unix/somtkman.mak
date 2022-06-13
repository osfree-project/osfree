#
#  Copyright 2012, Roger Brown
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

RELNOTES_DIR=$(INTDIR)/relnotes

all: $(RELNOTES_DIR)/ch1 $(RELNOTES_DIR)/ch2 $(RELNOTES_DIR)/ch3 $(RELNOTES_DIR)/ch4 $(RELNOTES_DIR)/TC

clean:
	$(CLEAN) $(RELNOTES_DIR)/ch* $(RELNOTES_DIR)/TC
	if test -d $(RELNOTES_DIR); then rmdir  $(RELNOTES_DIR); fi

dist:

install:

$(RELNOTES_DIR)/ch1:
	if test -f ../$(PLATFORM_PROTO)/ch1.txt; \
	then \
		mkdir -p $(RELNOTES_DIR); \
		rm -rf $@; \
		../../toolbox/txt2man.sh <../$(PLATFORM_PROTO)/ch1.txt >$@ ; \
		../../toolbox/manpack.sh $@ ; \
	fi

$(RELNOTES_DIR)/ch2:
	if test -f ../$(PLATFORM_PROTO)/ch2.txt; \
	then \
		mkdir -p $(RELNOTES_DIR); \
		rm -rf $@; \
		../../toolbox/txt2man.sh <../$(PLATFORM_PROTO)/ch2.txt >$@ ; \
		../../toolbox/manpack.sh $@ ; \
	fi

$(RELNOTES_DIR)/ch3:
	if test -f ../$(PLATFORM_PROTO)/ch3.txt; \
	then \
		mkdir -p $(RELNOTES_DIR); \
		rm -rf $@; \
		../../toolbox/txt2man.sh <../$(PLATFORM_PROTO)/ch3.txt >$@ ; \
		../../toolbox/manpack.sh $@ ; \
	fi

$(RELNOTES_DIR)/TC:
	if test -f ../$(PLATFORM_PROTO)/TC.txt; \
	then \
		mkdir -p $(RELNOTES_DIR); \
		cp ../$(PLATFORM_PROTO)/TC.txt $@; \
	fi

$(RELNOTES_DIR)/ch4: ../license.txt
	if test -f ../$(PLATFORM_PROTO)/TC.txt; \
	then \
		mkdir -p $(RELNOTES_DIR); \
		rm -rf $@; \
		../../toolbox/txt2man.sh <../license.txt >$@ ; \
		../../toolbox/manpack.sh $@ ; \
	fi

