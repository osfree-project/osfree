# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..$(SEP)include -dHAVE_LIBJPEG -dHAVE_LIBPNG -ecc

!include $(%ROOT)/mk/all.mk



all: $(ROOT)$(SEP)lib$(SEP)gdlib.lib .SYMBOLIC

OBJS = gd_jpeg.$(O) &
    gd_io_file.$(O) &
    gd_png.$(O) &
    gd.$(O) &
    gdfontg.$(O) &
    gdfontl.$(O) &
    gdfontmb.$(O) &
    gdfonts.$(O) &
    gdfontt.$(O) &
    gdhelpers.$(O) &
    gdtables.$(O) &
    gd_io.$(O) &
    gd_io_dp.$(O)



$(ROOT)$(SEP)lib$(SEP)gdlib.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
