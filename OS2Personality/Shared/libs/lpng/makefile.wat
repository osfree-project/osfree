# makefile
# Created by IBM WorkFrame/2 MakeMake at 16:23:48 on 29 Sept 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

!include ../../watcom.mif

#  $(sep)  -d2 
#sep=/
CC=wcc386
CFLAGS= -i$(zlib_dir) -i. -bm -dHAVE_LIBJPEG -dHAVE_LIBPNG $(LPNG_CFLAGS)
# -bm 

# '.obj' is changed to '$(OBJEXT)' 
# ' \' is changed to ' &'
# '.\' is changed to './'
# '{M:\Evgen\FreePM\lib\lpng}' is changed to '$(lpng_dir)'
# '{M:\Evgen\FreePM\lib\lpng;../Zlib;$(INCLUDE);}'   to  '$(lpng_dir)'
# 'M:\Evgen\FreePM\lib\lpng\'    to    ''

#lpng_dir=$(sep)pub$(sep)FreePM$(sep)FreePM$(sep)lib$(sep)lpng
#zlib_dir=$(sep)pub$(sep)FreePM$(sep)FreePM$(sep)lib$(sep)Zlib

.SUFFIXES:

.SUFFIXES: &
    $(OBJEXT) .c 

#.cpp.o :
#    echo Compiling $[@
#    wpp386  $(CFLAGS) -oaxt $[@

.c$(OBJEXT):
    @echo $[@
    #icc.exe /I..\Zlib /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s
    $(CC) $(CFLAGS) -oaxt $[@

#$(lpng_dir).c$(OBJEXT):
#    @echo " Compile::C++ Compiler 2"
    #icc.exe /I..\Zlib /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s

all: &
    .$(sep)example$(EXE_SUFFIX) 

clean: 
	-rm *.o *.obj lpng.lib example.exe example$(EXE_SUFFIX)

.$(sep)example$(EXE_SUFFIX): &
    .$(sep)pngwtran$(OBJEXT) &
    .$(sep)pngwutil$(OBJEXT) &
    .$(sep)png$(OBJEXT) &
    .$(sep)pngerror$(OBJEXT) &
    .$(sep)pnggccrd$(OBJEXT) &
    .$(sep)pngget$(OBJEXT) &
    .$(sep)pngmem$(OBJEXT) &
    .$(sep)pngread$(OBJEXT) &
    .$(sep)pngrio$(OBJEXT) &
    .$(sep)pngrtran$(OBJEXT) &
    .$(sep)pngrutil$(OBJEXT) &
    .$(sep)pngset$(OBJEXT) &
    .$(sep)pngtest$(OBJEXT) &
    .$(sep)pngtrans$(OBJEXT) &
    .$(sep)pngvcrd$(OBJEXT) &
    .$(sep)pngwio$(OBJEXT) &
    .$(sep)pngwrite$(OBJEXT) &
    ..$(sep)Zlib$(sep)Zlib.lib
    @echo " Link::Linker "
    #icc.exe @<<
    # /B" /exepack:2 /packd /optfunc /noe"
    # /Feexample.exe 
    #  
    wlib lpng.lib &
      .$(sep)pngwtran$(OBJEXT) &
      .$(sep)pngwutil$(OBJEXT) &
      .$(sep)png$(OBJEXT) &
      .$(sep)pngerror$(OBJEXT) &
      .$(sep)pnggccrd$(OBJEXT) &
      .$(sep)pngget$(OBJEXT) &
      .$(sep)pngmem$(OBJEXT) &
      .$(sep)pngread$(OBJEXT) &
      .$(sep)pngrio$(OBJEXT) &
      .$(sep)pngrtran$(OBJEXT) &
      .$(sep)pngrutil$(OBJEXT) &
      .$(sep)pngset$(OBJEXT) &
      .$(sep)pngtrans$(OBJEXT) &
      .$(sep)pngvcrd$(OBJEXT) &
      .$(sep)pngwio$(OBJEXT) &
      .$(sep)pngwrite$(OBJEXT)
    wlink system $(SYSTEM_TARGET) name example$(EXE_SUFFIX) &
     file ..$(sep)Zlib$(sep)Zlib.lib  &
     file .$(sep)pngwtran$(OBJEXT) &
     file .$(sep)pngwutil$(OBJEXT) &
     file .$(sep)png$(OBJEXT) &
     file .$(sep)pngerror$(OBJEXT) &
     file .$(sep)pnggccrd$(OBJEXT) &
     file .$(sep)pngget$(OBJEXT) &
     file .$(sep)pngmem$(OBJEXT) &
     file .$(sep)pngread$(OBJEXT) &
     file .$(sep)pngrio$(OBJEXT) &
     file .$(sep)pngrtran$(OBJEXT) &
     file .$(sep)pngrutil$(OBJEXT) &
     file .$(sep)pngset$(OBJEXT) &
     file .$(sep)pngtest$(OBJEXT) &
     file .$(sep)pngtrans$(OBJEXT) &
     file .$(sep)pngvcrd$(OBJEXT) &
     file .$(sep)pngwio$(OBJEXT) &
     file .$(sep)pngwrite$(OBJEXT)

#      file .$(sep)pngtest$(OBJEXT) &

.$(sep)pngwtran$(OBJEXT): &
    $(lpng_dir)$(sep)pngwtran.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngwrite$(OBJEXT): &
    $(lpng_dir)$(sep)pngwrite.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngwio$(OBJEXT): &
    $(lpng_dir)$(sep)pngwio.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngvcrd$(OBJEXT): &
    $(lpng_dir)$(sep)pngvcrd.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngtrans$(OBJEXT): &
    $(lpng_dir)$(sep)pngtrans.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngtest$(OBJEXT): &
    $(lpng_dir)$(sep)pngtest.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngset$(OBJEXT): &
    $(lpng_dir)$(sep)pngset.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngrutil$(OBJEXT): &
    $(lpng_dir)$(sep)pngrutil.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngrtran$(OBJEXT): &
    $(lpng_dir)$(sep)pngrtran.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngrio$(OBJEXT): &
    $(lpng_dir)$(sep)pngrio.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngread$(OBJEXT): &
    $(lpng_dir)$(sep)pngread.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngmem$(OBJEXT): &
    $(lpng_dir)$(sep)pngmem.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngget$(OBJEXT): &
    $(lpng_dir)$(sep)pngget.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pnggccrd$(OBJEXT): &
    $(lpng_dir)$(sep)pnggccrd.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngerror$(OBJEXT): &
    $(lpng_dir)$(sep)pngerror.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)png$(OBJEXT): &
    $(lpng_dir)$(sep)png.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h

.$(sep)pngwutil$(OBJEXT): &
    $(lpng_dir)$(sep)pngwutil.c &
    $(lpng_dir)$(sep)png.h &
    $(zlib_dir)$(sep)zlib.h &
    $(lpng_dir)$(sep)pngconf.h &
    $(zlib_dir)$(sep)zconf.h &
    $(lpng_dir)$(sep)alloc.h


