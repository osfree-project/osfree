###########################################################################
#  @(#)Makefile	1.15 
#
#  Makefile for building 'libdll.a', the library for building DLLs.
#  
#  Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
#
###########################################################################

include $(TWINDIR)/Makefile.config
TWIN_AUXINC	= -I$(TWINDIR)/win

#########################################################################

DLLOBJS = LibMain.o LibWEP.o DefTable.o ResTable.o DummyEnviron.o

#########################################################################

all: libdll$(TWIN_APISIZE).a


libdll$(TWIN_APISIZE).a:  $(DLLOBJS)
	${AR} libdll$(TWIN_APISIZE).a $(DLLOBJS)
	${RANLIB} libdll$(TWIN_APISIZE).a
	cp libdll$(TWIN_APISIZE).a $(TWINDIR)/win/libdll$(TWIN_APISIZE).a

install: $(TWINLIB)/libdll$(TWIN_APISIZE).a

install32: 
	$(MAKE) install TWIN_APISIZE=32

$(TWINLIB)/libdll$(TWIN_APISIZE).a: libdll$(TWIN_APISIZE).a 
	rm -f $(TWINLIB)/libdll$(TWIN_APISIZE).a
	cp libdll$(TWIN_APISIZE).a $(TWINLIB)/libdll$(TWIN_APISIZE).a
	chmod 755 $(TWINLIB)/libdll$(TWIN_APISIZE).a

clean:
	rm -rf $(DLLOBJS) libdll$(TWIN_APISIZE).a

clobber: clean

ctags:
	ctags *.c *.h >/dev/null 2>&1

depend:
	@-makedepend -f Makedepend.$(TARGET) $(DEPEND) -I. $(CFLAGS) LibMain.c LibWEP.c DefTable.c ResTable.c DummyEnviron.c 2>/dev/null

