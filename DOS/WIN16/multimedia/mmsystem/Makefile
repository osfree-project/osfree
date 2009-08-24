###############################################################################
#
# @(#)Makefile	1.6
#
# Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
#
# Makefile for building multimedia driver
#
###############################################################################

include $(TWINDIR)/Makefile.config

TWIN_AUXINC	= -I$(TWINDIR)/win -I$(TWINDIR)/multimedia/include
  
###############################################################################

MMOBJ	=	mmdrv.o mmaux.o midi.o wave.o

MMSRC	=	$(MMOBJ:.o=.c)

#########################################################################

all:	multimedia.o

nolink: $(MMOBJ)

multimedia.o: $(MMOBJ) $(TWINLIB)/multimedia.o
	$(LD) -r -o multimedia.o $(MMOBJ)
	cp multimedia.o $(TWINLIB)/multimedia.o

$(TWINLIB)/multimedia.o:

#########################################################################

ctags:
	ctags *.c *.h >/dev/null 2>&1

depend:
	@-makedepend -f Makedepend.$(TARGET) $(CCOPTS) $(MMSRC) 2>/dev/null

list:
	@for f in $(MMOBJ); do echo $$f;done

clean:	clobber

clobber: 	
	rm -rf $(MMOBJ) core multimedia.o

###############################################################################
#
# Source file dependencies are located in Makedepend.$(TARGET)
#
###############################################################################

include Makedepend.$(TARGET)

###############################################################################
# DO NOT DELETE THIS LINE -- make depend depends on it.

mmio.Osuf: /usr/include/ctype.h /usr/include/sys/stdsyms.h
