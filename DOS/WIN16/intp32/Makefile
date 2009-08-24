##########################################################################   
#
#	@(#)Makefile	1.19 
#
#	Copyright (c) 1995-1997, Willows Software Inc.  All rights reserved.
#
#	makefile for building 16/32 bit interpreter intp32 
#
##########################################################################   

include $(TWINDIR)/Makefile.config
TWIN_AUXINC	= -I$(TWINDIR)/win

##########################################################################   

SOURCE	=	fp87.c interp_main.c interp_modrm.c interp_16_32.c \
		interp_32_16.c interp_32_32.c

OBJECTS  =	fp87.o interp_main.o interp_modrm.o interp_16_32.o \
		interp_32_16.o interp_32_32.o

##########################################################################   

all: intp32.o libintp32.a

intp32.o: $(OBJECTS)
	$(LD) -r -o intp32.o $(OBJECTS)
	cp intp32.o $(TWINDIR)/win

libintp32.a:	$(OBJECTS)
		${AR} libintp32.a $?
		$(RANLIB) libintp32.a

clobber: clean

clean:
		rm -f $(OBJECTS)
		rm -f libintp32.a

source:
	supdate .

depend:
	makedepend -f Makedepend.$(TARGET) $(DEPEND) -I. $(CFLAGS)

##########################################################################
#
# Source file dependencies are located in Makedepend.$(TARGET)
#
include Makedepend.$(TARGET)

##########################################################################
#
#
# DO NOT DELETE THIS LINE -- make depend depends on it.
