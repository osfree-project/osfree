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

STDINCL=-I. -I..  -I../../include/$(PLATFORM)/$(BUILDTYPE) -I../../include 
SHLIBSUFFIX=.a
SHLIBPREFIX=lib
DLLPATHENV=LIBPATH
DLLPREFIX=
DLLSUFFIX=.dll
EXEPREFIX=
EXESUFFIX=
CC_EXE=$(CC) $(CFLAGS)
CC_DLL=$(CC) $(CFLAGS) $(CC_FLAG_FPIC)
CXX_EXE=$(CXX) $(CXXFLAGS)
CXX_DLL=$(CXX) $(CXXFLAGS) $(CXX_FLAG_FPIC)
STDCONFIGOPTS=-D_PLATFORM_UNIX_ 
STDOPT=$(PLATFORM_CFLAGS) $(MAKEDEFS_CFLAGS) $(STDCONFIGOPTS) 
STDOPTXX=$(PLATFORM_CXXFLAGS) $(MAKEDEFS_CXXFLAGS) $(STDCONFIGOPTS)
LDXCOFF_SH=../../toolbox/ldxcoff.sh
ASNEEDED_SH=../../toolbox/asneeded.sh
REFLIBS_SH=../../toolbox/reflibs.sh
LINKDLL=INTDIR="$(INTDIR)" $(REFLIBS_SH) $(LDXCOFF_SH) $(ASNEEDED_SH) $(LD) $(LDFLAGS) $(PLATFORM_LDFLAGS)
LINKDLLXX=INTDIR="$(INTDIR)" $(REFLIBS_SH) $(LDXCOFF_SH) $(ASNEEDED_SH) $(LD) $(LDFLAGS) $(PLATFORM_LDFLAGS)
LINKAPP=$(REFLIBS_SH) $(ASNEEDED_SH) $(CC) $(CFLAGS) $(PLATFORM_CFLAGS)
LINKAPPXX=$(REFLIBS_SH) $(ASNEEDED_SH) $(CXX) $(CXXFLAGS) $(PLATFORM_CXXFLAGS)
LINKALL_LIBPATH=/usr/lib:/lib
#  -bnoquiet
LINKDLL_HEAD=-G -blibpath:$(LINKALL_LIBPATH) -bnoautoexp -bernotok
LINKAPP_HEAD=-Wl,-blibpath:$(LINKALL_LIBPATH) -Wl,-brtl
