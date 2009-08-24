#
# A Makefile for Zlib
# (c) osFree project,
# valerius, 2006/10/30
#

PROJ = libz
ADD_COPT = -d__WATCOM__ -d__OS2__ -i=. -i=..$(SEP)..$(SEP)include
TARGETS  = $(PATH)$(PROJ).lib

srcfiles = $(p)adler32$(e) $(p)compress$(e) $(p)crc32$(e) $(p)gzio$(e) &
           $(p)uncompr$(e) $(p)deflate$(e) $(p)trees$(e) $(p)zutil$(e) &
           $(p)inflate$(e) $(p)inftrees$(e) &
           $(p)inffast$(e)

!include $(%ROOT)/mk/libsos2.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
