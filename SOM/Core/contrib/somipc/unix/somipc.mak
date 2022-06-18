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

OBJS=$(INTDIR)/rhbsc.o \
	 $(INTDIR)/rhbsckih.o \
	 $(INTDIR)/rhbidl.o \
	 $(INTDIR)/rhbscemt.o \
	 $(INTDIR)/rhbscir.o \
	 $(INTDIR)/rhbsctyp.o \
	 $(INTDIR)/rhbscpp.o \
	 $(INTDIR)/rhbschdr.o \
	 $(INTDIR)/rhbsctc.o \
	 $(INTDIR)/rhbscapi.o

TARGET=$(OUTDIR_BIN)/$(EXEPREFIX)somipc$(EXESUFFIX)

SOMIPC_INCL=-I../include -I../../somirfmt/include $(STDINCL)

all: $(TARGET) $(OUTDIR_MAN)/somipc.1

clean:
	$(CLEAN) $(OBJS) $(TARGET)  $(OUTDIR_MAN)/somipc.1 $(OUTDIR_MAN)/somipc.1.*

$(INTDIR)/rhbsc.o: ../src/rhbsc.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbsc.cpp -o $@

$(INTDIR)/rhbsctc.o: ../src/rhbsctc.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbsctc.cpp -o $@

$(INTDIR)/rhbscapi.o: ../src/rhbscapi.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbscapi.cpp -o $@

$(INTDIR)/rhbscc.o: ../src/rhbscc.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbscc.cpp -o $@

$(INTDIR)/rhbidl.o: ../src/rhbidl.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbidl.cpp -o $@

$(INTDIR)/rhbscpp.o: ../src/rhbscpp.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbscpp.cpp -o $@

$(INTDIR)/rhbsctyp.o: ../src/rhbsctyp.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbsctyp.cpp -o $@

$(INTDIR)/rhbscir.o: ../src/rhbscir.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbscir.cpp -o $@

$(INTDIR)/rhbscemt.o: ../src/rhbscemt.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbscemt.cpp -o $@

$(INTDIR)/rhbschdr.o: ../src/rhbschdr.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbschdr.cpp -o $@

$(INTDIR)/rhbscpdl.o: ../src/rhbscpdl.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbscpdl.cpp -o $@

$(INTDIR)/rhbsckih.o: ../src/rhbsckih.cpp
	$(CXX_EXE) $(STDOPTXX) $(SOMIPC_INCL) -c ../src/rhbsckih.cpp -o $@

$(TARGET): $(OBJS) 
	$(LINKAPPXX) $(LINKAPP_HEAD) $(OBJS) -o $@ $(LINKAPP_TAIL) 

dist install:

$(OUTDIR_MAN)/somipc.1: ../unix/manpage.txt
	../../toolbox/txt2man.sh <../unix/manpage.txt >$@
	../../toolbox/manpack.sh $@
