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

TARGET_EXE=$(OUTDIR_TESTS)/shlbtest$(EXESUFFIX)
TARGET_DLL=$(OUTDIR_SHLIB)/$(DLLPREFIX)shlb$(DLLSUFFIX)
OBJS_EXE=$(INTDIR)/shlbtest.o
OBJS_DLL=$(INTDIR)/shlb.o
SHLBTEST_H=$(INTDIR)/shlbtest.h
SHLB_H=$(INTDIR)/shlb.h

INCL=	-I$(INTDIR) 	\
		-I../../somkpub/include

all:  $(TARGET_DLL) $(TARGET_EXE) $(SHLBTEST_H)

clean:
	$(CLEAN) $(TARGET_DLL) $(TARGET_EXE) $(SHLBTEST_H) $(OBJS_EXE) $(OBJS_DLL) $(SHLB_H)

$(TARGET_DLL): $(OBJS_DLL)
	$(LINKDLL) $(LINKDLL_HEAD) 		\
		-o $@ 						\
		$(OBJS_DLL) 				\
		$(STDLIB) 					\
		`$(SHLB_EXP) shlbtest shlb` \
		`$(SHLB_ENT) SOMInitModule` \
		`$(SHLB_MAP) shlbtest shlb` \
		$(LINKDLL_TAIL)

$(SHLB_H):
	$(SHLB_DLO) -s shlb >$@

$(INTDIR)/shlb.o: ../src/shlb.c
	$(CC_DLL) $(STDOPT) $(INCL) $(STDINCL)  -c ../src/shlb.c -o $@

$(INTDIR)/shlbtest.o: ../src/shlbtest.c $(SHLB_H)
	$(CC_EXE) $(STDOPT) $(INCL) $(STDINCL)  -c ../src/shlbtest.c -DDLLPATHENV=\"$(DLLPATHENV)\" -o $@

$(TARGET_EXE): $(OBJS_EXE)
	$(LINKAPP) $(LINKAPP_HEAD) $(OBJS_EXE) -o $@ \
		`$(SHLB_MAP) shlbtest shlbtest` \
		$(STDLIB) $(LINKAPP_TAIL)

$(SHLBTEST_H): $(TARGET_EXE) $(TARGET_DLL)
	$(EXEC_CONFIG) $(HOSTDIR_TESTS)/shlbtest$(EXESUFFIX) -o $@

dist install:

