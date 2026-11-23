#
# A Makefile for Zlib
# (c) osFree project,
# valerius, 2006/10/30
#

PROJECT = libz
TARGET_CLASS = LIBRARY
TARGET_SUBCLASS = STATIC

ADD_COPT = -d__WATCOM__ -d__OS2__ -i=. -i=..$(SEP)..$(SEP)include

SOURCES  = adler32 crc32 deflate infback &
           inffast inflate inftrees trees zutil &
           compress uncompr gzclose gzlib gzread &
           gzwrite 

!include $(MYDIR)zlib.mk
!include $(%ROOT)tools/mk/build.mk

.c: $(PORT_BASE)$(SEP)zlib-1.4

.h: $(PORT_BASE)$(SEP)zlib-1.4


