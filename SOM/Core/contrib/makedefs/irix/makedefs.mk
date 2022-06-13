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

STDINCL=-I. -I.. -I../../include/$(PLATFORM)/$(BUILDTYPE) -I../../include/$(PLATFORM_PROTO) -I../../include
CC_EXE=$(CC) $(CFLAGS)
CC_DLL=$(CC) $(CFLAGS) 
CXX_EXE=$(CXX) $(CXXFLAGS)
CXX_DLL=$(CXX) $(CXXFLAGS) 
SHLIBPREFIX=lib
SHLIBSUFFIX=.so
DLLPREFIX=lib
DLLSUFFIX=.so
STDLIB=$(CONFIG_LIBS) -lc 
STDLIBXX=$(STDLIB)
COMMONOPTS=-D_PLATFORM_UNIX_ $(CONFIG_OPTS)
STDOPT=$(PLATFORM_CFLAGS) $(MAKEDEFS_CFLAGS) $(COMMONOPTS)
STDOPTXX=$(PLATFORM_CXXFLAGS) $(MAKEDEFS_CXXFLAGS) $(COMMONOPTS)
LINKDLL=CFLAGS="$(CFLAGS)" INTDIR="$(INTDIR)" ../../makedefs/$(PLATFORM_PROTO)/linktool.sh 
LINKDLLXX=CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" INTDIR="$(INTDIR)" ../../makedefs/$(PLATFORM_PROTO)/linktool.sh -cxx
LINKAPP=../../toolbox/asneeded.sh $(CC) $(CFLAGS)
LINKAPPXX=../../toolbox/asneeded.sh $(CXX) $(CXXFLAGS)
LINKDLL_HEAD=-shared
LINKDLL_TAIL=$(CXX_RUNTIME)
LINKAPP_HEAD=-Wl,-no_unresolved -Wl,-LD_MSG:error=157 `echo >$(INTDIR)/shlb_imp` $(CC_NO_UNDEFS)
LINKAPP_TAIL=`if test -f $(INTDIR)/shlb_imp; then rm $(INTDIR)/shlb_imp; fi`
ECHO=echo
