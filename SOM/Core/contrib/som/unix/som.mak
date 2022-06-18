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

TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)som$(DLLSUFFIX)
OBJS=$(INTDIR)/somkern.o \
	 $(INTDIR)/rhbsomid.o \
	 $(INTDIR)/somalloc.o \
	 $(INTDIR)/somobjva.o \
	 $(INTDIR)/somapi.o \
	 $(INTDIR)/sommutex.o \
	 $(INTDIR)/somkpath.o \
	 $(INTDIR)/somshlb.o

SOM_INCL=-I$(INTDIR) \
		 -I../include \
		 -I../src \
		 -I../../somcdr/include \
		 -I../../rhbmtut/include \
		 -I../../somcdr/src 	\
		 -I../../somidl/$(PLATFORM) \
		 -I../../somkpub/include \
		 -I../../somtk/include \
		 $(STDINCL)

CC_OPT=$(STDOPT) $(SOM_INCL) -DBUILD_SOM 

all: $(TARGET) $(IRS)

clean:
	$(CLEAN) $(OBJS) $(TARGET) $(INTDIR)/somirdll.h

$(TARGET): $(OBJS)
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@\
		$(OBJS) \
		`$(SHLB_EXP) som som` \
		`$(SHLB_ENT) somEnvironmentNew` \
		`$(ARLB_REF) rhbmtuta rhbmtuta` \
		`$(SHLB_MAP) som som` \
		$(STDLIB) \
		$(LINKDLL_TAIL)

$(INTDIR)/rhbsomid.o: ../../somid/src/rhbsomid.c 
	$(CC_DLL) $(CC_OPT) -c ../../somid/src/rhbsomid.c -o $@

$(INTDIR)/somalloc.o: ../src/somalloc.c 
	$(CC_DLL) $(CC_OPT) -c ../src/somalloc.c -o $@ 

$(INTDIR)/sommutex.o: ../src/sommutex.c 
	$(CC_DLL) $(CC_OPT) -c ../src/sommutex.c -o $@ 

$(INTDIR)/somapi.o: ../src/somapi.c 
	$(CC_DLL) $(CC_OPT) -c ../src/somapi.c -o $@ 

$(INTDIR)/somkpath.o: ../src/somkpath.c 
	$(CC_DLL) $(CC_OPT) -c ../src/somkpath.c -o $@ 

$(INTDIR)/somobjva.o: ../src/somobjva.c 
	$(CC_DLL) $(CC_OPT) -c ../src/somobjva.c -o $@ 

$(INTDIR)/somshlb.o: ../src/somshlb.c ../../shlbtest/$(PLATFORM)/$(BUILDTYPE)/shlbtest.h
	$(CC_DLL) $(CC_OPT) -I../../shlbtest/$(PLATFORM)/$(BUILDTYPE) -c ../src/somshlb.c -o $@ 

$(INTDIR)/somkern.o: ../src/somkern.c $(INTDIR)/somkernp.kih $(INTDIR)/somirdll.h ../src/somobj.c ../src/somcls.c ../src/somcm.c
	$(CC_DLL) $(CC_OPT) -c ../src/somkern.c -o $@ 

$(INTDIR)/somirdll.h: ../unix/som.mak 
	$(SHLB_DLO) -s somir >$@
	cat $@

dist install:

