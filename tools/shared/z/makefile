#
# A Makefile for Zlib
# (c) osFree project,
# valerius, 2006/10/30
#

srcfiles = $(p)adler32$(e) $(p)compress$(e) $(p)crc32$(e) $(p)gzio$(e) &
           $(p)uncompr$(e) $(p)deflate$(e) $(p)trees$(e) $(p)zutil$(e) &
           $(p)inflate$(e) $(p)infblock$(e) $(p)inftrees$(e) $(p)infcodes$(e) &
           $(p)infutil$(e) $(p)inffast$(e)

TARGETS = $(PATH)z.lib
DEST    = lib

!include $(%ROOT)/mk/tools.mk

# details see in $(ROOT)/mk/dirs.mk
#!define ABSOLUTE_DEST
#DEST    = $(TOOLDIR)lib$(SEP)

$(PATH)z.lib: $(OBJS)
 $(DC) $^@
 $(LIB) $(LIBOPT) $^@ +$(OBJS)

t: .SYMBOLIC
 $(SAY) $(OBJS)
