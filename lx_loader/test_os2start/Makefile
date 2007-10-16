##
## $Id: Makefile.in 172 2006-07-26 15:00:01Z sdi2 $
##

#srcdir = .
#top_srcdir = ../../..
#top_builddir = ../../..

#include $(top_srcdir)/Mk/l4.base.mk

LIBRARY = os2start
LIBRARY_DEPS = \
	loadobjlx.o \
	modlx.o \
	execlx.o \
	processlx.o \
	fixuplx.o \
	dynlink.o \
	native_dynlink.o \
	memmgr.o


# 	doscalls.o
# 	msg.o 
#	../msg.c 
#	../doscalls.c 
	
SRCS = \
	../loadobjlx.c \
	../modlx.c \
	../execlx.c \
	../processlx.c \
	../dynlink.c \
	../native_dynlink.c \
	../fixuplx.c \
	../memmgr.c


#IDLS = os2srv.idl
#os2srv.idl: $(top_srcdir)/if/iflocator.idl $(top_srcdir)/if/iffile.idl $(top_srcdir)/if/iffiledirectory.idl

#.o../.c:


		
VPATH += ..
# -DSDIOS
# The define TEST_OS2START is from when I compiled lxloader with SDIOS and Linux
# which needed different header files.

# An include file from Open Watcom (exe386.h) in /mnt/c/Projekt/osfree/src/include
# is also used. Don't remember if I modified it or not.
CFLAGS =  -Wall -DTEST_OS2START -g -I.. -I. -I/mnt/c/Projekt/osfree/src/include -pipe
CPPFLAGS = $(CFLAGS)

CC = g++
# tcc g++ $(CC)


LDFLAGS +=  -ldl

#LIBS += -lc -lsdi -ll4 -lio
#LDFLAGS += $(LIBRARY_LDFLAGS) -T $(top_builddir)/include/$(ARCH)/sdi.ld 

#include $(top_srcdir)/Mk/l4.lib.mk
#${LIBRARY_DEPS}:  $(SRCS) 
#	echo $<
	#$(CC) -c $(CFLAGS) $<
#.SUFFIXES:	
#.o .cc:
#	g++ -c $(CFLAGS) $<

#.o .c:
#	gcc -c $(CFLAGS) $<

.c.o:
	$(CC) $(CFLAGS) -c $<

test_os2start: test_os2start.o $(LIBRARY_DEPS)
	$(CC) $(CFLAGS) $(LIBRARY_DEPS) $(LDFLAGS) test_os2start.cc -o test_os2start

# A test to put the build date into an header file. Not used right now.
#build:
#	echo '#define BUILD "'`date`'"' > build.h

#   build
all:  test_os2start

clean: 
	rm -f $(LIBRARY_DEPS)

dist:
	sh ./create_tgz_archive.sh

test:
	$(MAKE) all -f Makefile_test_progs
