#
# A Makefile for Zlib
# (c) osFree project,
# valerius, 2006/10/30
#

#ADD_COPT = -d__WATCOM__ -d__OS2__ -i=. -i=..$(SEP)..$(SEP)include
#TARGETS  = $(PATH)$(PROJ).lib

SOURCES  = adler32 crc32 deflate infback &
           inffast inflate inftrees trees zutil &
           compress uncompr gzclose gzlib gzread &
           gzwrite 

TARGET_CLASS = LIBRARY
TARGET_SUBCLASS = STATIC
TARGET_API = HOST
TARGET_BITS = 32

!include $(%ROOT)tools/mk/build.mk
!include $(MYDIR)zlib.mk

.c: $(PORT_BASE)$(SEP)zlib-1.3.2

.h: $(PORT_BASE)$(SEP)zlib-1.3.2
