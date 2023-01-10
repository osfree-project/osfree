# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = gdlib
TRGT = $(PROJ).lib
ADD_COPT = -dHAVE_LIBJPEG -dHAVE_LIBPNG #-ecc
srcfiles = $(p)gd_jpeg$(e) $(p)gd_io_file$(e) $(p)gd_png$(e) $(p)gd$(e) $(p)gdfontg$(e) $(p)gdfontl$(e) &
           $(p)gdfontmb$(e) $(p)gdfonts$(e) $(p)gdfontt$(e) $(p)gdhelpers$(e) $(p)gdtables$(e) &
           $(p)gd_io$(e) $(p)gd_io_dp$(e)

!include $(%ROOT)tools/mk/libsos2.mk

TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
