# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = vorbis
ADD_COPT = -sg -i=$(MYDIR).. -i=$(MYDIR)..$(SEP)..$(SEP)include -i=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)libogg$(SEP)include
ADD_LINKOPT = lib clib3r.lib, os2386.lib, math3r.lib

srcfiles = &
 $(p)..$(SEP)mdct$(e) $(p)..$(SEP)smallft$(e) $(p)..$(SEP)block$(e) $(p)..$(SEP)envelope$(e) $(p)..$(SEP)window$(e) $(p)..$(SEP)lsp$(e) &
 $(p)..$(SEP)lpc$(e) $(p)..$(SEP)analysis$(e) $(p)..$(SEP)synthesis$(e) $(p)..$(SEP)psy$(e) $(p)..$(SEP)info$(e) &
 $(p)..$(SEP)floor1$(e) $(p)..$(SEP)floor0$(e) &
 $(p)..$(SEP)res0$(e) $(p)..$(SEP)mapping0$(e) $(p)..$(SEP)registry$(e) $(p)..$(SEP)codebook$(e) $(p)..$(SEP)sharedbook$(e) &
 $(p)..$(SEP)lookup$(e) $(p)..$(SEP)bitrate$(e) $(p)..$(SEP)vorbisfile$(e) $(p)..$(SEP)vorbisenc$(e)  $(p)..$(SEP)porting$(e)

DLL = 1
DLLOPT = initinstance
#STACKSIZE = 0xa0000000
IMPORTS = &
 oggpack_writeinit ogg.oggpack_writeinit, &
 oggpack_writeclear ogg.oggpack_writeclear, &
 oggpack_reset ogg.oggpack_reset, &
 oggpack_get_buffer ogg.oggpack_get_buffer, &
 oggpack_bytes ogg.oggpack_bytes, &
 oggpack_readinit ogg.oggpack_readinit, &
 oggpack_read ogg.oggpack_read, &
 oggpack_write ogg.oggpack_write, &
 oggpack_writealign ogg.oggpack_writealign, &
 oggpack_look ogg.oggpack_look, &
 oggpack_adv ogg.oggpack_adv, &
 oggpack_writetrunc ogg.oggpack_writetrunc, &
 ogg_sync_buffer ogg.ogg_sync_buffer, &
 ogg_sync_wrote ogg.ogg_sync_wrote, &
 ogg_sync_reset ogg.ogg_sync_reset, &
 ogg_sync_pageseek ogg.ogg_sync_pageseek, &
 ogg_page_serialno ogg.ogg_page_serialno, &
 ogg_stream_reset_serialno ogg.ogg_stream_reset_serialno, &
 ogg_stream_pagein ogg.ogg_stream_pagein, &
 ogg_stream_packetout ogg.ogg_stream_packetout, &
 ogg_page_granulepos ogg.ogg_page_granulepos, &
 ogg_sync_init ogg.ogg_sync_init, &
 ogg_stream_init ogg.ogg_stream_init, &
 ogg_stream_clear ogg.ogg_stream_clear, &
 ogg_sync_clear ogg.ogg_sync_clear, &
 ogg_page_eos ogg.ogg_page_eos, &
 ogg_stream_packetpeek ogg.ogg_stream_packetpeek, &
 ogg_page_continued ogg.ogg_page_continued, &
 ogg_page_bos ogg.ogg_page_bos, &
 ogg_stream_reset ogg.ogg_stream_reset

ALIASES = &
 oggpack_writeinit_=oggpack_writeinit, &
 oggpack_writeclear_=oggpack_writeclear, &
 oggpack_reset_=oggpack_reset, &
 oggpack_get_buffer_=oggpack_get_buffer, &
 oggpack_bytes_=oggpack_bytes, &
 oggpack_readinit_=oggpack_readinit, &
 oggpack_read_=oggpack_read, &
 oggpack_write_=oggpack_write, &
 oggpack_writealign_=oggpack_writealign, &
 oggpack_look_=oggpack_look, &
 oggpack_adv_=oggpack_adv, &
 oggpack_writetrunc_=oggpack_writetrunc, &
 ogg_sync_buffer_=ogg_sync_buffer, &
 ogg_sync_wrote_=ogg_sync_wrote, &
 ogg_sync_reset_=ogg_sync_reset, &
 ogg_sync_pageseek_=ogg_sync_pageseek, &
 ogg_page_serialno_=ogg_page_serialno, &
 ogg_stream_reset_serialno_=ogg_stream_reset_serialno, &
 ogg_stream_pagein_=ogg_stream_pagein, &
 ogg_stream_packetout_=ogg_stream_packetout, &
 ogg_page_granulepos_=ogg_page_granulepos, &
 ogg_sync_init_=ogg_sync_init, &
 ogg_stream_init_=ogg_stream_init, &
 ogg_stream_clear_=ogg_stream_clear, &
 ogg_sync_clear_=ogg_sync_clear, &
 ogg_page_eos_=ogg_page_eos, &
 ogg_stream_packetpeek_=ogg_stream_packetpeek, &
 ogg_page_continued_=ogg_page_continued, &
 ogg_page_bos_=ogg_page_bos, &
 ogg_stream_reset_=ogg_stream_reset

EXPORTS = &
  ov_streams = ov_streams_, &
  ov_comment = ov_comment_, &
  ov_bitrate = ov_bitrate_, &
  vorbis_book_encodev = vorbis_book_encodev_, &
  ov_pcm_seek_page_lap = ov_pcm_seek_page_lap_, &
  vorbis_granule_time = vorbis_granule_time_, &
  vorbis_book_codeword = vorbis_book_codeword_, &
  vorbis_analysis_init = vorbis_analysis_init_, &
  vorbis_comment_clear = vorbis_comment_clear_, &
  vorbis_bitrate_clear = vorbis_bitrate_clear_, &
  ov_time_seek_page_lap = ov_time_seek_page_lap_, &
  vorbis_invsq2explook = vorbis_invsq2explook_, &
  vorbis_lsp_to_curve = vorbis_lsp_to_curve_, &
  vorbis_comment_init = vorbis_comment_init_, &
  vorbis_bitrate_init = vorbis_bitrate_init_, &
  vorbis_lpc_from_data = vorbis_lpc_from_data_, &
  vorbis_info_blocksize = vorbis_info_blocksize_, &
  vorbis_comment_query = vorbis_comment_query_, &
  vorbis_synthesis_read = vorbis_synthesis_read_, &
  vorbis_analysis_wrote = vorbis_analysis_wrote_, &
  vorbis_synthesis_init = vorbis_synthesis_init_, &
  ov_seekable = ov_seekable_, &
  ov_halfrate = ov_halfrate_, &
  vorbis_analysis_buffer = vorbis_analysis_buffer_, &
  vorbis_bitrate_managed = vorbis_bitrate_managed_, &
  ov_time_seek = ov_time_seek_, &
  vorbis_staticbook_pack = vorbis_staticbook_pack_, &
  ov_crosslap = ov_crosslap_, &
  ov_time_tell = ov_time_tell_, &
  ov_pcm_seek = ov_pcm_seek_, &
  ov_pcm_tell = ov_pcm_tell_, &
  ov_raw_seek = ov_raw_seek_, &
  vorbis_encode_init_vbr = vorbis_encode_init_vbr_, &
  ov_raw_tell = ov_raw_tell_, &
  ov_test_open = ov_test_open_, &
  vorbis_comment_add_tag = vorbis_comment_add_tag_, &
  ov_pcm_total = ov_pcm_total_, &
  ov_halfrate_p = ov_halfrate_p_, &
  ov_read_float = ov_read_float_, &
  ov_raw_total = ov_raw_total_, &
  vorbis_window = vorbis_window_, &
  ov_time_total = ov_time_total_, &
  vorbis_book_decodev_add = vorbis_book_decodev_add_, &
  vorbis_book_init_decode = vorbis_book_init_decode_, &
  vorbis_packet_blocksize = vorbis_packet_blocksize_, &
  vorbis_book_init_encode = vorbis_book_init_encode_, &
  vorbis_book_decodevs_add = vorbis_book_decodevs_add_, &
  vorbis_book_decodevv_add = vorbis_book_decodevv_add_, &
  vorbis_staticbook_clear = vorbis_staticbook_clear_, &
  vorbis_book_decodev_set = vorbis_book_decodev_set_, &
  vorbis_coslook = vorbis_coslook_, &
  vorbis_bitrate_addblock = vorbis_bitrate_addblock_, &
  vorbis_commentheader_out = vorbis_commentheader_out_, &
  vorbis_staticbook_unpack = vorbis_staticbook_unpack_, &
  vorbis_synthesis_lapout = vorbis_synthesis_lapout_, &
  vorbis_synthesis_blockin = vorbis_synthesis_blockin_, &
  vorbis_synthesis_pcmout = vorbis_synthesis_pcmout_, &
  vorbis_encode_setup_vbr = vorbis_encode_setup_vbr_, &
  vorbis_analysis_blockout = vorbis_analysis_blockout_, &
  vorbis_synthesis_restart = vorbis_synthesis_restart_, &
  vorbis_synthesis_idheader = vorbis_synthesis_idheader_, &
  vorbis_synthesis_halfrate = vorbis_synthesis_halfrate_, &
  vorbis_encode_setup_init = vorbis_encode_setup_init_, &
  vorbis_analysis_headerout = vorbis_analysis_headerout_, &
  vorbis_synthesis_headerin = vorbis_synthesis_headerin_, &
  vorbis_staticbook_destroy = vorbis_staticbook_destroy_, &
  ov_time_seek_lap = ov_time_seek_lap_, &
  ov_serialnumber = ov_serialnumber_, &
  vorbis_analysis = vorbis_analysis_, &
  ov_pcm_seek_lap = ov_pcm_seek_lap_, &
  vorbis_bitrate_flushpacket = vorbis_bitrate_flushpacket_, &
  ov_raw_seek_lap = ov_raw_seek_lap_, &
  ov_pcm_seek_page = ov_pcm_seek_page_, &
  ov_open_callbacks = ov_open_callbacks_, &
  vorbis_info_init = vorbis_info_init_, &
  ov_time_seek_page = ov_time_seek_page_, &
  vorbis_dsp_clear = vorbis_dsp_clear_, &
  vorbis_synthesis_trackonly = vorbis_synthesis_trackonly_, &
  ov_test_callbacks = ov_test_callbacks_, &
  vorbis_synthesis = vorbis_synthesis_, &
  vorbis_comment_query_count = vorbis_comment_query_count_, &
  vorbis_invsqlook = vorbis_invsqlook_, &
  vorbis_book_clear = vorbis_book_clear_, &
  vorbis_info_clear = vorbis_info_clear_, &
  vorbis_encode_ctl = vorbis_encode_ctl_, &
  vorbis_block_init = vorbis_block_init_, &
  vorbis_fromdBlook = vorbis_fromdBlook_, &
  vorbis_lpc_to_lsp = vorbis_lpc_to_lsp_, &
  vorbis_book_decode = vorbis_book_decode_, &
  vorbis_book_encode = vorbis_book_encode_, &
  vorbis_block_clear = vorbis_block_clear_, &
  ov_read = ov_read_, &
  ov_clear = ov_clear_, &
  ov_open = ov_open_, &
  vorbis_encode_init = vorbis_encode_init_, &
  ov_info = ov_info_, &
  vorbis_lpc_predict = vorbis_lpc_predict_, &
  vorbis_encode_setup_managed = vorbis_encode_setup_managed_, &
  vorbis_synthesis_halfrate_p = vorbis_synthesis_halfrate_p_, &
  ov_fopen = ov_fopen_, &
  ov_test = ov_test_, &
  vorbis_comment_add = vorbis_comment_add_, &
  vorbis_book_errorv = vorbis_book_errorv_, &
  ov_bitrate_instant = ov_bitrate_instant_, &
  vorbis_book_codelen = vorbis_book_codelen_

DEST = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

TARGETS  = $(PATH)$(PROJ).dll

.c: $(MYDIR)..
