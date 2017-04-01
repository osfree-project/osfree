#
# A Makefile for Zlib
# (c) osFree project,
# valerius, 2006/10/30
#

PROJ = libz
TRGT = $(PROJ).lib
ADD_COPT = -d__WATCOM__ -d__OS2__ -i=. -i=..$(SEP)..$(SEP)include
TARGETS  = $(PATH)$(PROJ).lib

srcfiles = $(p)adler32$(e) $(p)crc32$(e) $(p)deflate$(e) $(p)infback$(e) &
           $(p)inffast$(e) $(p)inflate$(e) $(p)inftrees$(e) $(p)trees$(e) $(p)zutil$(e) &
           $(p)compress$(e) $(p)uncompr$(e) $(p)gzclose$(e) $(p)gzlib$(e) $(p)gzread$(e) &
           $(p)gzwrite$(e) 

# $(p)infblock$(e) $(p)infcodes$(e) $(p)gzio$(e) $(p)infutil$(e)

!include $(%ROOT)/mk/libsos2.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

OBJZ = adler32.o crc32.o deflate.o infback.o inffast.o inflate.o inftrees.o trees.o zutil.o
OBJG = compress.o uncompr.o gzclose.o gzlib.o gzread.o gzwrite.o
