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

# powerpc-apple-darwin
# i386-apple-darwin

STDINCL=-I . -I .. -I ../../include/$(PLATFORM)/$(BUILDTYPE) -I ../../include $(X11INCL)
CC_EXE=$(CC) $(CFLAGS)
CC_DLL=$(CC) $(CFLAGS) -fPIC
CXX_EXE=$(CXX) $(CXXFLAGS)
CXX_DLL=$(CXX) $(CXXFLAGS) -fPIC
SHLIBSUFFIX=.dylib
SHLIBPREFIX=lib
DLLSUFFIX=.dylib
DLLPREFIX=lib
DLLPATHENV=DYLD_LIBRARY_PATH
EXESUFFIX=
EXEPREFIX=
BUNDLESUFFIX=.bundle
BUNDLEPREFIX=
STDLIB=$(THREADLIBS) -lc $(DLLIBS)
STDLIBXX=$(THREADLIBS) -lc $(DLLIBS)
CONFIGOPTS=-Wall -Werror -D_PLATFORM_UNIX_ 
STDOPT=$(PLATFORM_CFLAGS) $(MAKEDEFS_CFLAGS) $(CONFIGOPTS) 
STDOPTXX=$(PLATFORM_CXXFLAGS) $(MAKEDEFS_CXXFLAGS) $(CONFIGOPTS) 
LINKDLL=CFLAGS="$(CFLAGS)" MAKEDEFS="$(MAKEDEFS)" ../../makedefs/$(PLATFORM_PROTO)/linktool.sh 
LINKDLLXX=CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)"  MAKEDEFS="$(MAKEDEFS)" ../../makedefs/$(PLATFORM_PROTO)/linktool.sh -cxx 
LINKAPP=../../makedefs/$(PLATFORM_PROTO)/linktool.sh 
LINKAPPXX=../../makedefs/$(PLATFORM_PROTO)/linktool.sh -cxx 
LINKDLL_HEAD=-shared
LINKDLL_TAIL=
LINKAPP_HEAD=
LINKAPP_TAIL=
