# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = gdlib
TRGT = $(PROJ).lib
ADD_COPT = -dHAVE_LIBJPEG -dHAVE_LIBPNG -dHAVE_INTTYPES_H -dfloorf=floor #-ecc

#$(p)gd_jpeg$(e) $(p)gd_io_file$(e) $(p)gd_png$(e) $(p)gd$(e) $(p)gdfontg$(e) $(p)gdfontl$(e) &
#           $(p)gdfontmb$(e) $(p)gdfonts$(e) $(p)gdfontt$(e) $(p)gdhelpers$(e) $(p)gdtables$(e) &
#           $(p)gd_io$(e) $(p)gd_io_dp$(e)

srcfiles = &
	$(p)gd$(e) &
	$(p)gd_avif$(e) &
	$(p)gd_bmp$(e) &
	$(p)gd_color$(e) &
	$(p)gd_color_map$(e) &
	$(p)gd_color_match$(e) &
	$(p)gd_crop$(e) &
	$(p)gd_filename$(e) &
	$(p)gd_filter$(e) &
	$(p)gd_gd$(e) &
	$(p)gd_gd2$(e) &
	$(p)gd_gif_in$(e) &
	$(p)gd_gif_out$(e) &
	$(p)gd_heif$(e) &
	$(p)gd_interpolation$(e) &
	$(p)gd_io$(e) &
	$(p)gd_io_dp$(e) &
	$(p)gd_io_file$(e) &
	$(p)gd_io_ss$(e) &
	$(p)gd_jpeg$(e) &
	$(p)gd_matrix$(e) &
	$(p)gd_nnquant$(e) &
	$(p)gd_png$(e) &
	$(p)gd_rotate$(e) &
	$(p)gd_security$(e) &
	$(p)gd_ss$(e) &
	$(p)gd_tga$(e) &
	$(p)gd_tiff$(e) &
	$(p)gd_topal$(e) &
	$(p)gd_transform$(e) &
	$(p)gd_version$(e) &
	$(p)gd_wbmp$(e) &
	$(p)gd_webp$(e) &
	$(p)gd_xbm$(e) &
	$(p)gdcache$(e) &
	$(p)gdfontg$(e) &
	$(p)gdfontl$(e) &
	$(p)gdfontmb$(e) &
	$(p)gdfonts$(e) &
	$(p)gdfontt$(e) &
	$(p)gdft$(e) &
	$(p)gdfx$(e) &
	$(p)gdhelpers$(e) &
	$(p)gdkanji$(e) &
	$(p)gdtables$(e) &
	$(p)gdxpm$(e) &
	$(p)wbmp$(e)

!include $(%ROOT)tools/mk/libsos2.mk
!include $(MYDIR)libgd.mk

.c: $(PORT_BASE)src

.h: $(PORT_BASE)src

TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
