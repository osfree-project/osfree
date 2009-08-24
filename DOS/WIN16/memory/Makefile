#########################################################################
#
# @(#)Makefile	1.29 3/6/97 05:54:42 /users/sccs/src/win/xdos/s.Makefile
#
#  Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
#
#  Makefile for building memory allocation module
#
#########################################################################

include $(TWINDIR)/Makefile.config

TWIN_AUXINC = -I$(TWINDIR)/win

#########################################################################
OBJECTS = VMAlloc.o 

all:	Memory.o

Memory.o: $(OBJECTS)
	${LD} -r -o Memory.o $(OBJECTS)
	cp Memory.o  $(TWINLIB)/Memory.o

#########################################################################

ctags:
	ctags *.c *.h >/dev/null 2>&1

depend:

clean:	clobber

clobber: 	
	rm -rf Memory.o $(OBJECTS)

##########################################################################
#
# Source file dependencies are located in Makedepend.$(TARGET)
#
##########################################################################

include Makedepend.$(TARGET)

##########################################################################
# DO NOT DELETE THIS LINE -- make depend depends on it.
