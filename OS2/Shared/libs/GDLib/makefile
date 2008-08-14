# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = gdlib
ADD_COPT = -i=..$(SEP)include -dHAVE_LIBJPEG -dHAVE_LIBPNG -ecc
TARGETS  = $(PATH)$(PROJ).lib
srcfiles = $(p)gd_jpeg$(e) $(p)gd_io_file$(e) $(p)gd_png$(e) $(p)gd$(e) $(p)gdfontg$(e) $(p)gdfontl$(e) &
           $(p)gdfontmb$(e) $(p)gdfonts$(e) $(p)gdfontt$(e) $(p)gdhelpers$(e) $(p)gdtables$(e) &
           $(p)gd_io$(e) $(p)gd_io_dp$(e)

!include $(%ROOT)/mk/libsos2.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
