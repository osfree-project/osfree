# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..\include

!include $(%ROOT)/mk/all.mk



all: $(ROOT)\lib\libtiff.lib .SYMBOLIC

OBJS = tif_aux.$(O) &
tif_close.$(O) &
tif_codec.$(O) &
tif_color.$(O) &
tif_compress.$(O) &
tif_dir.$(O) &
tif_dirinfo.$(O) &
tif_dirread.$(O) &
tif_dirwrite.$(O) &
tif_dumpmode.$(O) &
tif_error.$(O) &
tif_extension.$(O) &
tif_fax3.$(O) &
tif_fax3sm.$(O) &
tif_flush.$(O) &
tif_getimage.$(O) &
tif_jpeg.$(O) &
tif_luv.$(O) &
tif_lzw.$(O) &
tif_next.$(O) &
tif_open.$(O) &
tif_os2.$(O) &
tif_packbits.$(O) &
tif_pixarlog.$(O) &
tif_predict.$(O) &
tif_print.$(O) &
tif_read.$(O) &
tif_strip.$(O) &
tif_swab.$(O) &
tif_thunder.$(O) &
tif_tile.$(O) &
tif_version.$(O) &
tif_warning.$(O) &
tif_write.$(O) &
tif_zip.$(O)

$(ROOT)\lib\libtiff.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @wlib $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
