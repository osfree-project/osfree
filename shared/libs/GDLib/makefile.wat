# makefile
# Created by IBM WorkFrame/2 MakeMake at 14:11:01 on 12 Nov 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler.\
#  Lib::Library Tool

!include ../../watcom.mif

#  $(sep)  -d2 
#sep=/
CC= wcc386
CFLAGS= -i..$(sep)include -i. -dHAVE_LIBJPEG -dHAVE_LIBPNG $(GDLIB_CFLAGS)
# -bm 

# '.obj' is changed to '$(OBJEXT)' 

.SUFFIXES:

.SUFFIXES: &
    $(OBJEXT) .c 

.c$(OBJEXT):
    #icc.exe /DHAVE_LIBJPEG /DHAVE_LIBPNG /Ss /O /Gm /G5 /Gi /Oi /C %s
    @echo $[@
    $(CC)  $(CFLAGS) -oaxt $[@
    
#.cpp.o :
#    echo Compiling $[@
#    wpp386  $(CFLAGS) -oaxt $[@

#./.c$(OBJEXT):
#    @echo " Compile::C++ Compiler "
    #icc.exe /DHAVE_LIBJPEG /DHAVE_LIBPNG /Ss /O /Gm /G5 /Gi /Oi /C %s

all: &
    gdlib.lib
    
clean: 
	-rm *.o *.obj gdlib.lib

gdlib.lib: &
    .$(sep)gd_jpeg$(OBJEXT) &
    .$(sep)gd_io_file$(OBJEXT) &
    .$(sep)gd_png$(OBJEXT) &
    .$(sep)gd$(OBJEXT) &
    .$(sep)gdfontg$(OBJEXT) &
    .$(sep)gdfontl$(OBJEXT) &
    .$(sep)gdfontmb$(OBJEXT) &
    .$(sep)gdfonts$(OBJEXT) &
    .$(sep)gdfontt$(OBJEXT) &
    .$(sep)gdhelpers$(OBJEXT) &
    .$(sep)gdtables$(OBJEXT) &
    .$(sep)gd_io$(OBJEXT) &
    .$(sep)gd_io_dp$(OBJEXT)
    @echo " Lib::Library Tool "
    #ilib.exe  /nol gdlib.lib @makefile.rf;
    #wlink system os2v2_dll 
    wlib gdlib.lib .$(sep)gd_jpeg$(OBJEXT) &
     .$(sep)gd_io_file$(OBJEXT) &
     .$(sep)gd_png$(OBJEXT) &
     .$(sep)gd$(OBJEXT) &
     .$(sep)gdfontg$(OBJEXT) &
     .$(sep)gdfontl$(OBJEXT) &
     .$(sep)gdfontmb$(OBJEXT) &
     .$(sep)gdfonts$(OBJEXT) &
     .$(sep)gdfontt$(OBJEXT) &
     .$(sep)gdhelpers$(OBJEXT) &
     .$(sep)gdtables$(OBJEXT) &
     .$(sep)gd_io$(OBJEXT) &
     .$(sep)gd_io_dp$(OBJEXT)

.$(sep)gd_jpeg$(OBJEXT): &
    .$(sep)gd_jpeg.c &
    .$(sep)gd.h &
    .$(sep)gdhelpers.h &
    .$(sep)gd_io.h &
    .$(sep)jpeglib.h &
    .$(sep)jerror.h &
    .$(sep)jconfig.h &
    .$(sep)jmorecfg.h &
    .$(sep)jpegint.h

.$(sep)gd_io_file$(OBJEXT): &
    .$(sep)gd_io_file.c &
    .$(sep)gd.h &
    .$(sep)gdhelpers.h &
    .$(sep)gd_io.h

.$(sep)gd_io_dp$(OBJEXT): &
    .$(sep)gd_io_dp.c &
    .$(sep)gd.h &
    .$(sep)gdhelpers.h &
    .$(sep)gd_io.h

.$(sep)gd_io$(OBJEXT): &
    .$(sep)gd_io.c &
    .$(sep)gd.h &
    .$(sep)gd_io.h

.$(sep)gdtables$(OBJEXT): &
    .$(sep)gdtables.c

.$(sep)gdhelpers$(OBJEXT): &
    .$(sep)gdhelpers.c &
    .$(sep)gd.h &
    .$(sep)gdhelpers.h &
    .$(sep)gd_io.h

.$(sep)gdfontt$(OBJEXT): &
    .$(sep)gdfontt.c &
    .$(sep)gd.h &
    .$(sep)gd_io.h &
    .$(sep)gdfontt.h

.$(sep)gdfonts$(OBJEXT): &
    .$(sep)gdfonts.c &
    .$(sep)gd.h &
    .$(sep)gd_io.h &
    .$(sep)gdfonts.h

.$(sep)gdfontmb$(OBJEXT): &
    .$(sep)gdfontmb.c &
    .$(sep)gd.h &
    .$(sep)gd_io.h &
    .$(sep)gdfontmb.h

.$(sep)gdfontl$(OBJEXT): &
    .$(sep)gdfontl.c &
    .$(sep)gd.h &
    .$(sep)gd_io.h &
    .$(sep)gdfontl.h

.$(sep)gdfontg$(OBJEXT): &
    .$(sep)gdfontg.c &
    .$(sep)gd.h &
    .$(sep)gd_io.h &
    .$(sep)gdfontg.h

.$(sep)gd$(OBJEXT): &
    .$(sep)gd.c &
    .$(sep)gd.h &
    .$(sep)gdhelpers.h &
    .$(sep)gd_io.h

.$(sep)gd_png$(OBJEXT): &
    .$(sep)gd_png.c &
    .$(sep)gd.h &
    .$(sep)gdhelpers.h &
    .$(sep)png.h &
    .$(sep)gd_io.h &
    .$(sep)zlib.h &
    .$(sep)pngconf.h &
    .$(sep)zconf.h &
    .$(sep)alloc.h
