##
## $Id: Makefile.in 172 2006-07-26 15:00:01Z sdi2 $
##

#srcdir = .
#top_srcdir = ../../..
#top_builddir = ../../..

#include $(top_srcdir)/Mk/l4.base.mk

#CC = owcc -x c++
CC = g++

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
# is also used. Don't remember if I modified it or not.  -pipe
# OW needs __USE_MISC on Linux and -mabi=c -fm

WATCOM_CFLAGS= -mabi=c -fm -I/mnt/c/Projekt/open_watcom_src_1.6/rel2/lh -D__USE_MISC 
CFLAGS =  -Wall -DTEST_OS2START -g -I.. -I. -I/mnt/c/Projekt/osfree/src/include 
CPPFLAGS = $(CFLAGS)

#CC = g++
# tcc g++ $(CC)

#  /usr/lib/libdl.a
# For GCC
# LDFLAGS +=  -ldl

# For Open Watcom /usr/lib/libdl.a
LDFLAGS += -L/usr/lib -ldl 

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

.cc.o:
	$(CC) $(CFLAGS) -c $<

WATCOM_LINK = -Wl,'import dlsym_,dlopen_,dlerror_'
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
