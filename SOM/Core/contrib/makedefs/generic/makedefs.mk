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

STDINCL=-I../../include/$(PLATFORM)/$(BUILDTYPE) -I../../rhbseh/include -I../../include $(X11INCL)
SOCKLIBS= 
SHLIBSUFFIX=.a
SHLIBPREFIX=lib
DLLSUFFIX=.a
DLLPREFIX=lib
DLLPATHENV=PATH
STDLIB=$(CONFIG_LIBS)
STDSALIBS=
STDLIBXX=$(CONFIG_LIBS)
STDOPTALL=-Wall -Werror -D_PLATFORM_UNIX_ -D_ALL_SOURCE -DBUILD_STATIC
STDOPT=$(PLATFORM_CFLAGS) $(MAKEDEFS_CFLAGS) $(STDOPTALL)
STDOPTXX=$(PLATFORM_CXXFLAGS) $(MAKEDEFS_CXXFLAGS) $(STDOPTALL)
LINKDLL=AR=$(AR) RANLIB=$(RANLIB) LIBPREFIX=lib LIBSUFFIX=.a  ../../toolbox/makeliba.sh
LINKDLLXX=AR=$(AR) RANLIB=$(RANLIB) LIBPREFIX=lib LIBSUFFIX=.a  ../../toolbox/makeliba.sh
LINKAPP=AR=$(AR) RANLIB=$(RANLIB) LIBPREFIX=lib LIBSUFFIX=.a ../../toolbox/adlib.sh ../../toolbox/collect.sh $(CC) $(CFLAGS)
LINKAPPXX=AR=$(AR) RANLIB=$(RANLIB) LIBPREFIX=lib LIBSUFFIX=.a ../../toolbox/adlib.sh ../../toolbox/collect.sh $(CXX) $(CXXFLAGS)
LINKDLL_HEAD= 
LINKDLL_TAIL=
LINKAPP_HEAD= 
LINKAPP_TAIL=`$(SHLB_REF) rhbseh rhbseh` -lc
CONFIG_DEFS=
CC_EXE=$(CC) $(CFLAGS)
CC_DLL=$(CC) $(CFLAGS) 
CXX_EXE=$(CXX) $(CXXFLAGS) 
CXX_DLL=$(CXX) $(CXXFLAGS) 
