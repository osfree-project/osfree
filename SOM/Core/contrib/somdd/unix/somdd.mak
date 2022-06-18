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

SOMDD_INCL=	-I$(INTDIR)								\
			-I../include							\
			-I../../somcdr/$(PLATFORM)/$(INTDIR)	\
			-I../../somcdr/include					\
			-I../../rhbmtut/include					\
			-I../../somidl/$(PLATFORM)				\
			-I../../rhbexecd/include				\
			-I../../somkpub/include					\
			-I../../somtk/include					\
			$(STDINCL) 

OBJS=	$(INTDIR)/rhbsomdd.o \
		$(INTDIR)/rhbexec.o \
		$(INTDIR)/rhbexecd.o \
		$(INTDIR)/implreps.o \
		$(INTDIR)/somddsrv.o \
		$(INTDIR)/somddevm.o \
		$(INTDIR)/somddtc.o \
		$(INTDIR)/somddrep.o \
		$(INTDIR)/somddior.o # $(INTDIR)/somddexe.o $(INTDIR)/rhbsomkd.o $(INTDIR)/rhbsomup.o

TARGET=$(OUTDIR_BIN)/$(EXEPREFIX)somdd$(EXESUFFIX)

all: $(TARGET) $(OUTDIR_MAN)/somdd.8 

clean:
	$(CLEAN) $(OBJS) $(TARGET) $(INTDIR)/somddsrv.tc $(INTDIR)/somdsvr.h $(OUTDIR_MAN)/somdd.8 $(OUTDIR_MAN)/somdd.8.*

$(INTDIR)/somddsrv.tc: ../somddsrv.idl 
	$(SC) -p -stc -D __GENERATE_SOMDD__ -I.. -I../../somidl -d $(INTDIR) ../somddsrv.idl

$(INTDIR)/somdsvr.h:
	echo "static char sz_somdsvr[]=\"$(EXEPREFIX)somdsvr$(EXESUFFIX)\";" >$@
	cat $@

$(INTDIR)/rhbsomdd.o: ../src/rhbsomdd.c $(INTDIR)/somdsvr.h
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL)  -DBUILD_SOMDD -c ../src/rhbsomdd.c -o $@

$(INTDIR)/rhbexec.o: ../src/rhbexec.c $(INTDIR)/somdsvr.h
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL)  -DBUILD_SOMDD -c ../src/rhbexec.c -o $@

$(INTDIR)/rhbexecd.o: ../../rhbexecd/src/rhbexecd.c 
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL)  -DBUILD_SOMDD -c ../../rhbexecd/src/rhbexecd.c -o $@

$(INTDIR)/rhbsomkd.o: ../../somcdr/src/rhbsomkd.c
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL)  -DBUILD_SOMDD -c ../../somcdr/src/rhbsomkd.c -o $@

$(INTDIR)/implreps.o: ../src/implreps.c
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL)  -DBUILD_SOMDD -c ../src/implreps.c -o $@

$(INTDIR)/somddsrv.o: ../src/somddsrv.c
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL)  -DBUILD_SOMDD -c ../src/somddsrv.c -o $@

$(INTDIR)/somddevm.o: ../src/somddevm.c
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL)  -DBUILD_SOMDD -c ../src/somddevm.c -o $@

$(INTDIR)/somddrep.o: ../src/somddrep.c
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL)  -DBUILD_SOMDD -c ../src/somddrep.c -o $@

$(INTDIR)/somddior.o: ../src/somddior.c
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL)  -DBUILD_SOMDD -c ../src/somddior.c -o $@

$(INTDIR)/rhbsomup.o: ../../somcdr/src/rhbsomup.c
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL)  -DBUILD_SOMDD -c ../../somcdr/src/rhbsomup.c -o $@

$(INTDIR)/somddtc.o: ../src/somddtc.c $(INTDIR)/somddsrv.tc
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL) -I../../somtc  -DBUILD_SOMDD -c ../src/somddtc.c -o $@

$(INTDIR)/somddexe.o: ../src/somddexe.c
	$(CC_EXE) $(STDOPT) $(SOMDD_INCL)  -DBUILD_SOMDD -c ../src/somddexe.c -o $@

$(TARGET): $(OBJS)
	$(LINKAPP) $(LINKAPP_HEAD) $(OBJS) -o $@ \
		`$(SHLB_REF) somd somd` \
		`$(SHLB_REF) somdcomm somdcomm` \
		`$(SHLB_REF) somcdr somcdr` \
		`$(SHLB_REF) somu somu` \
		`$(SHLB_REF) somir somir` \
		`$(SHLB_REF) somestrm somestrm` \
		`$(SHLB_REF) somem somem` \
		`$(SHLB_REF) soms soms` \
		`$(SHLB_REF) somtc somtc` \
		`$(SHLB_REF) somref somref` \
		`$(SHLB_REF) som som` \
		`$(SHLB_REF) somos somos` \
		`$(ARLB_REF) rhbmtuta rhbmtuta` \
		`$(SHLB_MAP) somdd somdd` \
		$(UUIDLIBS) $(SOCKLIBS) $(STDLIB)  $(LINKAPP_TAIL)

$(OUTDIR_BIN)/somdd_r: ../src/somddexe.c
	$(CC_EXE) -I.. ../src/somddexe.c -o $@ $(STDSALIBS)


dist install:

$(OUTDIR_MAN)/somdd.8: ../unix/manpage.txt
	../../toolbox/txt2man.sh <../unix/manpage.txt >$@
	../../toolbox/manpack.sh $@
