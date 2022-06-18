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

PLATDEFS=../../makedefs/$(PLATFORM_PROTO)/makedefs.mk
ENDIAN_SH=../unix/endian.sh
ENDIAN_H=../../include/rhbendia.h
RHBENDIA_C=../src/rhbendia.c
OBJS=$(INTDIR)/rhbendia.o
CONFIG_INCLUDE=../../include/$(PLATFORM)/$(BUILDTYPE)
TARGET=$(OUTDIR_TESTS)/rhbendia$(EXESUFFIX)
CONFIG2_SH=../unix/config2.sh
CONFIG3_SH=../unix/config3.sh
CONFIG2_C=../src/config2.c
CONFIG2_CPP=$(INTDIR)/config2.cpp
CONFIG_H=$(CONFIG_INCLUDE)/config.h
CONFIG_HPP=$(CONFIG_INCLUDE)/config.hpp
CONFIG_MK=$(INTDIR)/config.mk
CONFIG2_C_LOG=$(INTDIR)/config2c.log
CONFIG2_CPP_LOG=$(INTDIR)/config2x.log
EXECUTOR_SH=$(OUTDIR_TOOLS)/executor.sh

all:  test $(CONFIG_H) $(CONFIG_HPP) $(CONFIG_MK)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(PLATFORM_CFLAGS) $(RHBENDIA_O) -o $@ $(OBJS)

$(INTDIR)/rhbendia.o: $(RHBENDIA_C) $(ENDIAN_H)
	$(CC) $(CFLAGS) $(PLATFORM_CFLAGS) -I`dirname $(ENDIAN_H)` -o $@ -c $(RHBENDIA_C)

test: $(TARGET) $(EXECUTOR_SH) $(CONFIG_INCLUDE) 
	$(EXEC_TEST) $(TARGET)

../../include/$(PLATFORM):
	if test ! -d $@; then mkdir $@; fi

$(CONFIG_INCLUDE): ../../include/$(PLATFORM)
	if test ! -d $@; then mkdir $@; fi

$(CONFIG_H): $(CONFIG2_SH) $(CONFIG2_C)
	chmod +x $(CONFIG2_SH)
	CONFIG_H=$@ \
	CONFIG_LOG="$(CONFIG2_C_LOG)" \
	CONFIG2_C="$(CONFIG2_C)" \
	EXESUFFIX="$(EXESUFFIX)" \
	INTDIR="$(INTDIR)" \
	CC="$(CC)" \
	"$(CONFIG2_SH)" $(CONFIG_HEAD) \
	$(CFLAGS) $(CONFIG_CFLAGS) $(STDOPT) $(STDINCL) $(XMLIBS) $(XTLIBS) $(X11LIBS) $(PCSCLIBS) $(UUIDLIBS) $(CFLIBS) $(ATLIBS) $(SSLLIBS) $(CRYPTOLIBS) $(SOCKLIBS) $(STDLIB) $(CONFIG_TAIL)

$(CONFIG_HPP): $(CONFIG2_SH) $(CONFIG2_CPP)
	chmod +x "$(CONFIG2_SH)"
	CONFIG_H="$@" \
	CONFIG_LOG="$(CONFIG2_CPP_LOG)" \
	CONFIG2_C="$(CONFIG2_CPP)" \
	EXESUFFIX="$(EXESUFFIX)" \
	INTDIR="$(INTDIR)" \
	CC="$(CXX)" \
	"$(CONFIG2_SH)" $(CONFIG_HEAD) \
	$(CXXFLAGS) $(CONFIG_CXXFLAGS) $(STDOPTXX) $(STDINCL) $(XMLIBS) $(XTLIBS) $(X11LIBS) $(PCSCLIBS) $(UUIDLIBS) $(CFLIBS) $(ATLIBS) $(SSLLIBS) $(CRYPTOLIBS) $(SOCKLIBS) $(STDLIBXX) $(CONFIG_TAIL)

$(CONFIG2_CPP): $(CONFIG2_C)
	if test -f $@; then rm $@; fi
	cp $(CONFIG2_C) $@

$(CONFIG_MK): ../unix/config.mak $(ENDIAN_H) $(ENDIAN_SH) $(MAKEDEFS) $(PLATDEFS)
	chmod +x $(ENDIAN_SH)
	chmod +x $(CONFIG3_SH)
	if test "$(MAKEDEFS_ORIG)" = ""; then \
	ENDIAN=`CFLAGS="$(CFLAGS) -I\`dirname $(ENDIAN_H)\`" CC="$(CC)" INTDIR="$(INTDIR)" $(ENDIAN_SH)` ; \
	echo ENDIAN is $$ENDIAN ; \
	CFLAGS="$(CFLAGS)" \
	CXXFLAGS="$(CXXFLAGS)" \
	MAKEDEFS="$(MAKEDEFS)" \
	CONFIG_H="$(CONFIG_H)" \
	CONFIG_HPP="$(CONFIG_HPP)" \
	INTDIR="$(INTDIR)" \
	$(CONFIG3_SH) \
	`if test "$$ENDIAN" != ""; then echo "-D$$ENDIAN"; fi` >$@ ; fi
	cat $@

clean:
	$(CLEAN) \
		$(CONFIG_MK) \
		$(CONFIG_H) \
		$(CONFIG_HPP) \
		$(TARGET) \
		$(CONFIG2_CPP) \
		$(CONFIG2_CPP_LOG) \
		$(CONFIG2_C_LOG) \
		$(INTDIR)/config2.lst \
		$(OBJS) \
		$(EXECUTOR_SH)

crash:
	exit 1

$(EXECUTOR_SH):
	echo "#!/bin/sh" >$@
	echo "$(DLLPATHENV)"=$(OUTDIR_SHLIB):$(OUTDIR_IMPLIB):"$$""$(DLLPATHENV)" "$$""@" >>$@
	chmod +x $@

dist install:

