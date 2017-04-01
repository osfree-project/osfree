# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = ogg
TRGT = $(PROJ).dll
ADD_COPT = $(defines) -i=$(MYDIR) -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT = lib clib3r.lib, os2386.lib # $(PATH)..$(SEP)libogg.lib

OBJS = $(PATH)..$(SEP)framing.$(O) $(PATH)..$(SEP)bitwise.$(O)
DLL = 1
DLLOPT = initinstance
EXPORTS = &
# From framing.obj
  ogg_stream_packetin = ogg_stream_packetin_, &
  ogg_stream_flush = ogg_stream_flush_, &
  ogg_sync_pageseek = ogg_sync_pageseek_, &
  ogg_stream_pagein = ogg_stream_pagein_, &
  ogg_stream_pageout = ogg_stream_pageout_, &
  ogg_sync_init = ogg_sync_init_, &
  ogg_sync_clear = ogg_sync_clear_, &
  ogg_sync_reset = ogg_sync_reset_, &
  ogg_sync_destroy = ogg_sync_destroy_, &
  ogg_sync_buffer = ogg_sync_buffer_, &
  ogg_sync_wrote = ogg_sync_wrote_, &
  ogg_sync_pageout = ogg_sync_pageout_, &
  ogg_stream_packetout = ogg_stream_packetout_, &
  ogg_stream_packetpeek = ogg_stream_packetpeek_, &
  ogg_stream_init = ogg_stream_init_, &
  ogg_stream_clear = ogg_stream_clear_, &
  ogg_stream_reset = ogg_stream_reset_, &
  ogg_stream_reset_serialno = ogg_stream_reset_serialno_, &
  ogg_stream_destroy = ogg_stream_destroy_, &
  ogg_stream_eos = ogg_stream_eos_, &
  ogg_page_checksum_set = ogg_page_checksum_set_, &
  ogg_page_version = ogg_page_version_, &
  ogg_page_continued = ogg_page_continued_, &
  ogg_page_bos = ogg_page_bos_, &
  ogg_page_eos = ogg_page_eos_, &
  ogg_page_granulepos = ogg_page_granulepos_, &
  ogg_page_serialno = ogg_page_serialno_, &
  ogg_page_pageno = ogg_page_pageno_, &
  ogg_page_packets = ogg_page_packets_, &
  ogg_packet_clear = ogg_packet_clear_, &
# From bitwise.obj
  oggpack_write = oggpack_write_, &
  oggpack_read = oggpack_read_, &
  oggpack_writeinit = oggpack_writeinit_, &
  oggpack_writetrunc = oggpack_writetrunc_, &
  oggpack_writealign = oggpack_writealign_, &
  oggpack_writecopy = oggpack_writecopy_, &
  oggpack_reset = oggpack_reset_, &
  oggpack_writeclear = oggpack_writeclear_, &
  oggpack_readinit = oggpack_readinit_, &
  oggpack_look = oggpack_look_, &
  oggpack_look1 = oggpack_look1_, &
  oggpack_adv = oggpack_adv_, &
  oggpack_adv1 = oggpack_adv1_, &
  oggpack_read1 = oggpack_read1_, &
  oggpack_bytes = oggpack_bytes_, &
  oggpack_bits = oggpack_bits_, &
  oggpack_get_buffer = oggpack_get_buffer_

DEST = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

TARGETS  = $(PATH)$(PROJ).dll
