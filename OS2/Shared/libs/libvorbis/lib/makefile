# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = libvorbis
TRGT = $(PROJ).lib
DIRS = dll
ADD_COPT = -i=$(MYDIR) -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)..$(SEP)libogg$(SEP)include

srcfiles = &
 $(p)mdct$(e) $(p)smallft$(e) $(p)block$(e) $(p)envelope$(e) $(p)window$(e) $(p)lsp$(e) &
 $(p)lpc$(e) $(p)analysis$(e) $(p)synthesis$(e) $(p)psy$(e) $(p)info$(e) &
 $(p)floor1$(e) $(p)floor0$(e) &
 $(p)res0$(e) $(p)mapping0$(e) $(p)registry$(e) $(p)codebook$(e) $(p)sharedbook$(e) &
 $(p)lookup$(e) $(p)bitrate$(e) $(p)vorbisfile$(e) $(p)vorbisenc$(e)  $(p)porting$(e)

!include $(%ROOT)/mk/libsos2.mk

TARGETS  = $(PATH)$(PROJ).lib subdirs

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

qwe: .symbolic
 @echo $(ADD_COPT)
