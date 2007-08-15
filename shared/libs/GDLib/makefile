# makefile
# Created by IBM WorkFrame/2 MakeMake at 14:11:01 on 12 Nov 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Lib::Library Tool

.SUFFIXES:

.SUFFIXES: \
    .c .obj 

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DHAVE_LIBJPEG /DHAVE_LIBPNG /Ss /O /Gm /G5 /Gi /Oi /C %s

{M:\Evgen\FreePM\lib\GDLib}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DHAVE_LIBJPEG /DHAVE_LIBPNG /Ss /O /Gm /G5 /Gi /Oi /C %s

all: \
    .\gdlib.lib

.\gdlib.lib: \
    .\gd_jpeg.obj \
    .\gd_io_file.obj \
    .\gd_png.obj \
    .\gd.obj \
    .\gdfontg.obj \
    .\gdfontl.obj \
    .\gdfontmb.obj \
    .\gdfonts.obj \
    .\gdfontt.obj \
    .\gdhelpers.obj \
    .\gdtables.obj \
    .\gd_io.obj \
    .\gd_io_dp.obj
    @echo " Lib::Library Tool "
    ilib.exe  /nol gdlib.lib @makefile.rf;

.\gd_jpeg.obj: \
    M:\Evgen\FreePM\lib\GDLib\gd_jpeg.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gdhelpers.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}jpegint.h

.\gd_io_file.obj: \
    M:\Evgen\FreePM\lib\GDLib\gd_io_file.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gdhelpers.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h

.\gd_io_dp.obj: \
    M:\Evgen\FreePM\lib\GDLib\gd_io_dp.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gdhelpers.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h

.\gd_io.obj: \
    M:\Evgen\FreePM\lib\GDLib\gd_io.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h

.\gdtables.obj: \
    M:\Evgen\FreePM\lib\GDLib\gdtables.c

.\gdhelpers.obj: \
    M:\Evgen\FreePM\lib\GDLib\gdhelpers.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gdhelpers.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h

.\gdfontt.obj: \
    M:\Evgen\FreePM\lib\GDLib\gdfontt.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gdfontt.h

.\gdfonts.obj: \
    M:\Evgen\FreePM\lib\GDLib\gdfonts.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gdfonts.h

.\gdfontmb.obj: \
    M:\Evgen\FreePM\lib\GDLib\gdfontmb.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gdfontmb.h

.\gdfontl.obj: \
    M:\Evgen\FreePM\lib\GDLib\gdfontl.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gdfontl.h

.\gdfontg.obj: \
    M:\Evgen\FreePM\lib\GDLib\gdfontg.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gdfontg.h

.\gd.obj: \
    M:\Evgen\FreePM\lib\GDLib\gd.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gdhelpers.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h

.\gd_png.obj: \
    M:\Evgen\FreePM\lib\GDLib\gd_png.c \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gdhelpers.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}gd_io.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\GDLib;$(INCLUDE);}alloc.h
