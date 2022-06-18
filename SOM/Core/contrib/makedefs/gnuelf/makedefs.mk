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

STDINCL=-I . -I .. -I ../../include/$(PLATFORM)/$(BUILDTYPE) -I ../../include $(X11INCL)
CC_EXE=$(CC) $(CFLAGS)
CC_DLL=$(CC) $(CFLAGS) $(CC_FLAG_FPIC)
CXX_EXE=$(CXX) $(CXXFLAGS)
CXX_DLL=$(CXX) $(CXXFLAGS) $(CXX_FLAG_FPIC)
SHLIBPREFIX=lib
SHLIBSUFFIX=.so
DLLPREFIX=lib
DLLSUFFIX=.so
DLLPATHENV=LD_LIBRARY_PATH
STDLIB=$(CONFIG_LIBS)
STDLIBXX=$(CONFIG_LIBS)
COMMONOPTS=-Wall -Werror -D_PLATFORM_UNIX_ $(CONFIG_OPTS)
STDOPT=$(PLATFORM_CFLAGS) $(MAKEDEFS_CFLAGS) $(COMMONOPTS)
STDOPTXX=$(PLATFORM_CXXFLAGS) $(MAKEDEFS_CXXFLAGS) $(COMMONOPTS)
LINKDLL=INTDIR="$(INTDIR)" \
		MAKEDEFS="$(MAKEDEFS)" \
		CFLAGS="$(CFLAGS)" \
		PLATFORM_CFLAGS="$(PLATFORM_CFLAGS)" \
		../../makedefs/$(PLATFORM_PROTO)/linktool.sh 
LINKDLLXX=INTDIR="$(INTDIR)" \
		MAKEDEFS="$(MAKEDEFS)" \
		CFLAGS="$(CFLAGS)" \
		CXXFLAGS="$(CXXFLAGS)" \
		PLATFORM_CFLAGS="$(PLATFORM_CFLAGS)" \
		PLATFORM_CXXFLAGS="$(PLATFORM_CXXFLAGS)" \
		../../makedefs/$(PLATFORM_PROTO)/linktool.sh -cxx
LINKAPP=LIBPREFIX=$(DLLPREFIX) LIBSUFFIX=$(DLLSUFFIX) ../../toolbox/adlib.sh ../../toolbox/asneeded.sh $(CC) $(CFLAGS)
LINKAPPXX=LIBPREFIX=$(DLLPREFIX) LIBSUFFIX=$(DLLSUFFIX) ../../toolbox/adlib.sh ../../toolbox/asneeded.sh $(CXX) $(CXXFLAGS)
LINKDLL_HEAD=-shared
LINKDLL_TAIL=-lc
LINKAPP_HEAD=-Wl,-rpath-link,$(OUTDIR_SHLIB) `echo >$(INTDIR)/shlb_imp` $(CC_NO_UNDEFS)
LINKAPP_TAIL=`if test -f $(INTDIR)/shlb_imp; then rm $(INTDIR)/shlb_imp; fi`
