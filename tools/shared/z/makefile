#
# A Makefile for Zlib
# (c) osFree project,
# valerius, 2006/10/30
#

srcfiles = $(p)adler32$(e) $(p)compress$(e) $(p)crc32$(e) $(p)gzio$(e) &
           $(p)uncompr$(e) $(p)deflate$(e) $(p)trees$(e) $(p)zutil$(e) &
           $(p)inflate$(e) $(p)infblock$(e) $(p)inftrees$(e) $(p)infcodes$(e) &
           $(p)infutil$(e) $(p)inffast$(e)

#DEST      = ..$(SEP)build$(SEP)lib
TARGETS = $(PATH)z.lib

!include $(%ROOT)/mk/libs.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
