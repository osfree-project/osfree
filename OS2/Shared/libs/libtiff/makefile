# Makefile for common shared libs

PROJ = libtiff
TRGT = $(PROJ).lib
ADD_COPT = -i=..$(SEP)include

srcfiles = $(p)tif_aux$(e) $(p)tif_close$(e) $(p)tif_codec$(e) $(p)tif_color$(e) $(p)tif_compress$(e) &
           $(p)tif_dir$(e) $(p)tif_dirinfo$(e) $(p)tif_dirread$(e) $(p)tif_dirwrite$(e) $(p)tif_dumpmode$(e) &
           $(p)tif_error$(e) $(p)tif_extension$(e) $(p)tif_fax3$(e) $(p)tif_fax3sm$(e) $(p)tif_flush$(e) &
           $(p)tif_getimage$(e) $(p)tif_jpeg$(e) $(p)tif_luv$(e) $(p)tif_lzw$(e) $(p)tif_next$(e) $(p)tif_open$(e) &
           $(p)tif_os2$(e) $(p)tif_packbits$(e) $(p)tif_pixarlog$(e) $(p)tif_predict$(e) $(p)tif_print$(e) &
           $(p)tif_read$(e) $(p)tif_strip$(e) $(p)tif_swab$(e) $(p)tif_thunder$(e) $(p)tif_tile$(e) $(p)tif_version$(e) &
           $(p)tif_warning$(e) $(p)tif_write$(e) $(p)tif_zip$(e)

!include $(%ROOT)tools/mk/libsos2.mk

TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
