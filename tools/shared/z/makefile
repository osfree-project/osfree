#
# A Makefile for Zlib
# (c) osFree project,
# valerius, 2006/10/30
#

SOURCES  = adler32 crc32 deflate infback &
           inffast inflate inftrees trees zutil &
           compress uncompr gzclose gzlib gzread &
           gzwrite 

TARGET_CLASS = LIBRARY

!include $(%ROOT)tools/mk/build.mk
!include $(MYDIR)zlib.mk

.c: $(PORT_BASE)$(SEP)zlib-1.3.2

.h: $(PORT_BASE)$(SEP)zlib-1.3.2
