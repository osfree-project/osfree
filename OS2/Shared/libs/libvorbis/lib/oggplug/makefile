# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = OggPlay
ADD_COPT = -sg -i=$(MYDIR)include -i=$(MYDIR).. -i=$(MYDIR)..$(SEP)..$(SEP)include -i=$(MYDIR)..$..$(SEP)..$(SEP)..$(SEP)..$(SEP)libogg$(SEP)include
ADD_LINKOPT = lib clib3r.lib, os2386.lib, libvorbis.lib, libogg.lib &
              segment DATA nonshared readwrite

srcfiles = &
 $(p)command$(e) $(p)oggplay$(e)

DLL = 1
DLLOPT = initinstance terminstance
#STACKSIZE = 0xa0000000

EXPORTS = &
    decoder_init, &
    decoder_uninit, &
    decoder_command, &
    decoder_status, &
    decoder_length, &
    decoder_fileinfo, &
    decoder_support, &
    decoder_trackinfo, &
    decoder_cdinfo, &
    plugin_query

DEST = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

TARGETS  = $(PATH)$(PROJ).dll

.c: $(MYDIR)..
