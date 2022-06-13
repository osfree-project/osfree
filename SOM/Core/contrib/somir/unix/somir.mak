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

TARGET=$(OUTDIR_SHLIB)/$(DLLPREFIX)somir$(DLLSUFFIX)
TARGET_IMPLIB=$(OUTDIR_IMPLIB)/$(DLLPREFIX)somir$(DLLSUFFIX)

OBJS=	$(INTDIR)/somirfmt.o \
		$(INTDIR)/rhbsomir.o \
		$(INTDIR)/operatdf.o \
		$(INTDIR)/intfacdf.o \
		$(INTDIR)/repostry.o \
		$(INTDIR)/moduledf.o \
		$(INTDIR)/typedef.o \
		$(INTDIR)/attribdf.o \
		$(INTDIR)/containd.o \
		$(INTDIR)/containr.o \
		$(INTDIR)/excptdef.o \
		$(INTDIR)/paramdef.o \
		$(INTDIR)/constdef.o 

all: $(TARGET)

clean:
	$(CLEAN) $(OBJS) $(TARGET)

SOMIR_INCL= -I$(INTDIR) \
			-I../include \
			-I../../somirfmt/include \
			-I../../somtc/include \
			-I../../somipc/include \
			-I../../somidl/$(PLATFORM) \
			-I../../rhbmtut/include	\
			-I../../somkpub/include \
			-I../../somtk/include \
			-I../../rhbiniut/include \
			$(STDINCL)

$(TARGET): $(OBJS)  
	$(LINKDLL) $(LINKDLL_HEAD) \
		-o $@ \
		$(OBJS) \
		`$(SHLB_EXP) somir somir` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(ARLB_REF) rhbmtuta rhbmtuta` \
		`$(SHLB_REF) som som` \
		`$(SHLB_REF) somtc somtc` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_MAP) somir somir` \
		$(STDLIB) \
		$(LINKDLL_TAIL)

$(INTDIR)/somirfmt.o: ../../somirfmt/src/somirfmt.c
	$(CC_DLL) $(STDOPT)  -DBUILD_SOMIR $(SOMIR_INCL) -c ../../somirfmt/src/somirfmt.c -o $@

$(INTDIR)/rhbiniut.o: ../../rhbiniut/src/rhbiniut.c
	$(CC_DLL) $(STDOPT)  -DBUILD_SOMIR $(SOMIR_INCL) -c ../../rhbiniut/src/rhbiniut.c -o $@

$(INTDIR)/rhbsomir.o: ../src/rhbsomir.c
	$(CC_DLL) $(STDOPT)  -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/rhbsomir.c -o $@

$(INTDIR)/rhbsomkd.o: ../../somcdr/src/rhbsomkd.c
	$(CC_DLL) $(STDOPT)  -DBUILD_SOMIR $(SOMIR_INCL) -c ../../somcdr/src/rhbsomkd.c -o $@

$(INTDIR)/operatdf.o: ../src/operatdf.c
	$(CC_DLL) $(STDOPT)  -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/operatdf.c -o $@

$(INTDIR)/intfacdf.o: ../src/intfacdf.c
	$(CC_DLL) $(STDOPT)  -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/intfacdf.c -o $@

$(INTDIR)/repostry.o: ../src/repostry.c
	$(CC_DLL) $(STDOPT)  -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/repostry.c -o $@

$(INTDIR)/moduledf.o: ../src/moduledf.c
	$(CC_DLL) $(STDOPT)  -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/moduledf.c -o $@

$(INTDIR)/typedef.o: ../src/typedef.c
	$(CC_DLL) $(STDOPT)  -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/typedef.c -o $@

$(INTDIR)/constdef.o: ../src/constdef.c
	$(CC_DLL) $(STDOPT) -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/constdef.c -o $@

$(INTDIR)/containd.o: ../src/containd.c
	$(CC_DLL) $(STDOPT) -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/containd.c -o $@

$(INTDIR)/containr.o: ../src/containr.c
	$(CC_DLL) $(STDOPT) -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/containr.c -o $@

$(INTDIR)/attribdf.o: ../src/attribdf.c
	$(CC_DLL) $(STDOPT) -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/attribdf.c -o $@

$(INTDIR)/excptdef.o: ../src/excptdef.c
	$(CC_DLL) $(STDOPT) -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/excptdef.c -o $@

$(INTDIR)/paramdef.o: ../src/paramdef.c
	$(CC_DLL) $(STDOPT) -DBUILD_SOMIR $(SOMIR_INCL) -c ../src/paramdef.c -o $@


dist install:

