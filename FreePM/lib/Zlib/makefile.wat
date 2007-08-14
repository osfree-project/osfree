# makefile
# Created by IBM WorkFrame/2 MakeMake at 15:24:14 on 29 Sept 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Lib::Library Tool

!include ../../watcom.mif

#  $(sep)  -d2 
#sep=/
CPP= wpp386
CC=wcc386
CFLAGS= -i..$(sep)include -i. $(ZLIB_CFLAGS)
# -bm 

# '.obj' is changed to '$(OBJEXT)' 
# ' \' is changed to ' &'
# '.\' is changed to './'
# '{M:\Evgen\FreePM\utils\Zlib}' is changed to '$(zlib_dir)'
# '{M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}'   to  '$(zlib_dir)/'
# 'M:\Evgen\FreePM\utils\Zlib\'    to    '$(zlib_dir)/'

#zlib_dir=$(sep)pub$(sep)FreePM$(sep)FreePM$(sep)lib$(sep)Zlib


.SUFFIXES:

.SUFFIXES: &
     $(OBJEXT) .c

.c$(OBJEXT):
    #icc.exe /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s
    @echo $[@
    wcc386  $(CFLAGS) -oaxt $[@
    

#$(zlib_dir).c$(OBJEXT):
#    @echo " Compile::C++ Compiler "
    #icc.exe /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s

#.cpp.o :
#    echo Compiling $[@
#    wpp386  $(CFLAGS) -oaxt $[@

all: &
    .$(sep)Zlib.lib

clean: 
	-rm *.o *.obj Zlib.lib

.$(sep)Zlib.lib: &
    .$(sep)uncompr$(OBJEXT) &
    .$(sep)trees$(OBJEXT) &
    .$(sep)zutil$(OBJEXT) &
    .$(sep)adler32$(OBJEXT) &
    .$(sep)compress$(OBJEXT) &
    .$(sep)crc32$(OBJEXT) &
    .$(sep)deflate$(OBJEXT) &
    .$(sep)gzio$(OBJEXT) &
    .$(sep)infblock$(OBJEXT) &
    .$(sep)infcodes$(OBJEXT) &
    .$(sep)inffast$(OBJEXT) &
    .$(sep)inflate$(OBJEXT) &
    .$(sep)inftrees$(OBJEXT) &
    .$(sep)infutil$(OBJEXT)
    @echo " Lib::Library Tool "
    #ilib.exe  /nol Zlib.lib @makefile.rf;
    wlib Zlib.lib .$(sep)uncompr$(OBJEXT) &
     .$(sep)trees$(OBJEXT) &
     .$(sep)zutil$(OBJEXT) &
     .$(sep)adler32$(OBJEXT) &
     .$(sep)compress$(OBJEXT) &
     .$(sep)crc32$(OBJEXT) &
     .$(sep)deflate$(OBJEXT) &
     .$(sep)gzio$(OBJEXT) &
     .$(sep)infblock$(OBJEXT) &
     .$(sep)infcodes$(OBJEXT) &
     .$(sep)inffast$(OBJEXT) &
     .$(sep)inflate$(OBJEXT) &
     .$(sep)inftrees$(OBJEXT) &
     .$(sep)infutil$(OBJEXT)

.$(sep)uncompr$(OBJEXT): &
    $(zlib_dir)$(sep)uncompr.c &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h

.$(sep)trees$(OBJEXT): &
    $(zlib_dir)$(sep)trees.c &
    $(zlib_dir)$(sep)zutil.h &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h &
    $(zlib_dir)$(sep)deflate.h &
    $(zlib_dir)$(sep)trees.h

.$(sep)infutil$(OBJEXT): &
    $(zlib_dir)$(sep)infutil.c &
    $(zlib_dir)$(sep)zutil.h &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h &
    $(zlib_dir)$(sep)infblock.h &
    $(zlib_dir)$(sep)inftrees.h &
    $(zlib_dir)$(sep)infcodes.h &
    $(zlib_dir)$(sep)infutil.h

.$(sep)inftrees$(OBJEXT): &
    $(zlib_dir)$(sep)inftrees.c &
    $(zlib_dir)$(sep)zutil.h &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h &
    $(zlib_dir)$(sep)inftrees.h &
    $(zlib_dir)$(sep)inffixed.h

.$(sep)inflate$(OBJEXT): &
    $(zlib_dir)$(sep)inflate.c &
    $(zlib_dir)$(sep)zutil.h &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h &
    $(zlib_dir)$(sep)infblock.h

.$(sep)inffast$(OBJEXT): &
    $(zlib_dir)$(sep)inffast.c &
    $(zlib_dir)$(sep)zutil.h &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h &
    $(zlib_dir)$(sep)infblock.h &
    $(zlib_dir)$(sep)inftrees.h &
    $(zlib_dir)$(sep)infcodes.h &
    $(zlib_dir)$(sep)infutil.h &
    $(zlib_dir)$(sep)inffast.h

.$(sep)infcodes$(OBJEXT): &
    $(zlib_dir)$(sep)infcodes.c &
    $(zlib_dir)$(sep)zutil.h &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h &
    $(zlib_dir)$(sep)infblock.h &
    $(zlib_dir)$(sep)inftrees.h &
    $(zlib_dir)$(sep)infcodes.h &
    $(zlib_dir)$(sep)infutil.h &
    $(zlib_dir)$(sep)inffast.h

.$(sep)infblock$(OBJEXT): &
    $(zlib_dir)$(sep)infblock.c &
    $(zlib_dir)$(sep)zutil.h &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h &
    $(zlib_dir)$(sep)infblock.h &
    $(zlib_dir)$(sep)inftrees.h &
    $(zlib_dir)$(sep)infcodes.h &
    $(zlib_dir)$(sep)infutil.h

.$(sep)gzio$(OBJEXT): &
    $(zlib_dir)$(sep)gzio.c &
    $(zlib_dir)$(sep)zutil.h &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h

.$(sep)deflate$(OBJEXT): &
    $(zlib_dir)$(sep)deflate.c &
    $(zlib_dir)$(sep)zutil.h &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h &
    $(zlib_dir)$(sep)deflate.h

.$(sep)crc32$(OBJEXT): &
    $(zlib_dir)$(sep)crc32.c &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h

.$(sep)compress$(OBJEXT): &
    $(zlib_dir)$(sep)compress.c &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h

.$(sep)adler32$(OBJEXT): &
    $(zlib_dir)$(sep)adler32.c &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h

.$(sep)zutil$(OBJEXT): &
    $(zlib_dir)$(sep)zutil.c &
    $(zlib_dir)$(sep)zutil.h &
    $(zlib_dir)$(sep)zlib.h &
    $(zlib_dir)$(sep)zconf.h
