#########################################################################
#
# @(#)Makefile	1.29 3/6/97 05:54:42 /users/sccs/src/win/xdos/s.Makefile
#
#  Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
#
#  Makefile for building dos emulation library.
#
#########################################################################

include $(TWINDIR)/Makefile.config

TWIN_AUXINC = -I$(TWINDIR)/win

#########################################################################

XDOSOBJ = \
	dos_int86.o dos_call.o dos_create.o dos_date.o \
	dos_error.o dos_timer.o dos_findfile.o\
	int_21.o fn_36.o fn_38.o \
	int_10.o int_11.o int_12.o int_13.o int_15.o int_1a.o \
	int_25.o int_2a.o int_2f.o int_31.o int_3d.o int_5c.o \
	mfs_config.o mfs_fileio.o mfs_findfile.o mfs_regexp.o \
	msclib.o mfs_core.o fat_core.o fat_fileio.o fat_findfile.o \
	fat_lib.o

XDOSSRC = \
	dos_int86.c dos_call.c dos_create.c dos_date.c \
	dos_error.c dos_timer.c dos_findfile.c\
	int_21.c fn_36.c fn_38.c \
	int_10.c int_11.c int_12.c int_15.c int_1a.c \
	int_25.c int_2f.c int_31.c int_3d.c int_5c.c \
	mfs_config.c mfs_fileio.c mfs_findfile.c mfs_regexp.c \
	msclib.c mfs_core.c fat_core.c fat_fileio.c fat_findfile.c \
	fat_lib.c

XDOSHDR = \
	dosdefn.h ftype.h xdos.h \
	xtime.h xdosproc.h


#########################################################################

all:	xdos.o

nolink: $(XDOSOBJ)

xdos.o: $(XDOSOBJ) 
	${LD} -r -o xdos.o $(XDOSOBJ)
	cp xdos.o  $(TWINLIB)/xdos.o

$(TWINLIB)/xdos.o:

#########################################################################

ctags:
	ctags *.c *.h >/dev/null 2>&1

depend:
	@-makedepend -f Makedepend.$(TARGET) -I. $(CFLAGS) $(XDOSSRC) 2>/dev/null

clean:	clobber

clobber: 	
	rm -rf xdos.o $(XDOSOBJ)

##########################################################################
#
# Source file dependencies are located in Makedepend.$(TARGET)
#
##########################################################################

include Makedepend.$(TARGET)

##########################################################################
# DO NOT DELETE THIS LINE -- make depend depends on it.
