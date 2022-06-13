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

STDINCL=-I. -I.. -I../../include/$(PLATFORM)/$(BUILDTYPE) -I../../include -I../../rhbseh/include $(X11INCL)
CC_FLAG_FPIC=-fPIC
CXX_FLAG_FPIC=-fPIC
CC_EXE=$(CC) $(CFLAGS)
CC_DLL=$(CC) $(CFLAGS) $(CC_FLAG_FPIC)
CXX_EXE=$(CXX) $(CXXFLAGS)
CXX_DLL=$(CXX) $(CXXFLAGS) $(CXX_FLAG_FPIC)
SHLIBPREFIX=lib
SHLIBSUFFIX=.so
DLLPREFIX=lib
DLLSUFFIX=.so
DLLPATHENV=LD_LIBRARY_PATH
X11LIBS=-L/usr/X11R6/lib -lXmu -lXext -lXt -lX11
SOCKLIBS= 
COMMONLIBS=`$(SHLB_REF) rhbseh rhbseh`
STDLIB=$(COMMONLIBS)
STDLIBXX=$(COMMONLIBS)
COMMONOPTS=-Wall -Werror -D_PLATFORM_UNIX_ 
STDOPT=$(PLATFORM_CFLAGS) $(MAKEDEFS_CFLAGS) $(COMMONOPTS)
STDOPTXX=$(PLATFORM_CXXFLAGS) $(MAKEDEFS_CXXFLAGS) $(COMMONOPTS)
LINKDLL=../../makedefs/$(PLATFORM_PROTO)/linktool.sh -shared
LINKDLLXX=../../makedefs/$(PLATFORM_PROTO)/linktool.sh -cxx -shared
LINKAPP=$(CC) $(CFLAGS)
LINKAPPXX=$(CXX) $(CXXFLAGS)
LINKDLL_HEAD=
LINKDLL_TAIL=-lc
LINKAPP_HEAD=`echo >$(INTDIR)/shlb_imp` -Wl,-Bdynamic
LINKAPP_TAIL=`if test -f $(INTDIR)/shlb_imp; then rm $(INTDIR)/shlb_imp; fi`
