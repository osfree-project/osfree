# Makefile for Libtiff 3.8.2 (static)

# This makefile is for Open Watcom C++ on OS/2.
# It builds the Libtiff library as a statically linkable library (.LIB).

32_BITS = 

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk
!include $(%ROOT)/mk/all.mk

# Modify this line to point to the zlib library 1.2.3 or higher
ZLIB = ..$(SEP)zlib.123
IJG  = ..$(SEP)jpeg-6b

# set directory where binaries will be placed by target install
IDIR = bin

# include configuration setting for nmake (except compiler options)
# !INCLUDE nmake.opt

#
# Using Open Watcom C++:
#
CC      = wcc386
CL      = wlink
CLIB    = wlib

# Builds libtiff objects which are compiled multithreaded
# Therefore all users should also be multithreaded
CWARNS     = -wx -we
CFLAGS     = $(CWARNS) -i$(ZLIB) -i$(IJG) $(EXTRAFLAGS) -bd -zq -zp4 -onatxh -oe=50 -sg -ei -6r -fp5 -bm -mf /DNDEBUG
CFLAGS_DBG = $(CWARNS) -i$(ZLIB) -i$(IJG) $(EXTRAFLAGS) -bd -zq -zp4 -sg -ei -6r -fp5 -bm -mf -d2 /DDEBUG

LIBFLAGS     = -q -n -b -c

# For debugging call nmake or wmake with debug=on
!ifdef debug
CFLAGS   = $(CFLAGS_DBG)
!endif

#.SUFFIXES:	.c .obj

#.c.obj:
#		$(CC) $(CFLAGS) $*.c

#

OBJS_SYSDEP_MODULE = tif_os2.obj

OBJS =  tif_aux.obj &
	tif_close.obj &
	tif_codec.obj &
	tif_color.obj &
	tif_compress.obj &
	tif_dir.obj &
	tif_dirinfo.obj &
	tif_dirread.obj &
	tif_dirwrite.obj &
	tif_dumpmode.obj &
	tif_error.obj &
	tif_extension.obj &
	tif_fax3.obj &
	tif_fax3sm.obj &
	tif_getimage.obj &
	tif_jpeg.obj &
	tif_flush.obj &
	tif_luv.obj &
	tif_lzw.obj &
	tif_next.obj &
	tif_open.obj &
	tif_packbits.obj &
	tif_pixarlog.obj &
	tif_predict.obj &
	tif_print.obj &
	tif_read.obj &
	tif_swab.obj &
	tif_strip.obj &
	tif_thunder.obj &
	tif_tile.obj &
	tif_version.obj &
	tif_warning.obj &
	tif_write.obj &
	tif_zip.obj &
	$(OBJS_SYSDEP_MODULE)

#

all:   tiff.lib .SYMBOLIC


tiff.lib: $(OBJS)
          $(CLIB) $(LIBFLAGS) $@ +tif_aux.obj +tif_close.obj +tif_codec.obj +tif_color.obj &
                                 +tif_compress.obj +tif_dir.obj +tif_dirinfo.obj +tif_dirread.obj &
                                 +tif_dirwrite.obj +tif_dumpmode.obj +tif_error.obj +tif_extension.obj &
                                 +tif_fax3.obj +tif_fax3sm.obj +tif_getimage.obj +tif_jpeg.obj &
                                 +tif_flush.obj +tif_luv.obj +tif_lzw.obj +tif_next.obj +tif_open.obj &
                                 +tif_packbits.obj +tif_pixarlog.obj +tif_predict.obj +tif_print.obj &
                                 +tif_read.obj +tif_swab.obj +tif_strip.obj +tif_thunder.obj &
                                 +tif_tile.obj +tif_version.obj +tif_warning.obj +tif_write.obj &
                                 +tif_zip.obj +$(OBJS_SYSDEP_MODULE)


# ------------------------
# Build management targets
# ------------------------

clean:
         -del /N $(OBJS) *.lst 2> nul

clobber:
         -del /N $(OBJS) *.lst tiff.lib $(IDIR)\* 2> nul

install:
         copy tiff.lib     $(IDIR)
         copy tiff.h       $(IDIR)
         copy tiffio.h     $(IDIR)
         copy tiffconf.h   $(IDIR)
         copy tif_conf.h   $(IDIR)

package:
         -del /N $(IDIR)\libtiff382.zip
         zip -9 $(IDIR)\libtiff382.zip tiff.lib tiff.h tiffio.h tiffconf.h tif_conf.h

