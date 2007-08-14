# makefile
# Created by IBM WorkFrame/2 MakeMake at 15:24:14 on 29 Sept 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Lib::Library Tool

.SUFFIXES:

.SUFFIXES: \
    .c .obj 

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s

{M:\Evgen\FreePM\utils\Zlib}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s

all: \
    .\Zlib.lib

.\Zlib.lib: \
    .\uncompr.obj \
    .\trees.obj \
    .\zutil.obj \
    .\adler32.obj \
    .\compress.obj \
    .\crc32.obj \
    .\deflate.obj \
    .\gzio.obj \
    .\infblock.obj \
    .\infcodes.obj \
    .\inffast.obj \
    .\inflate.obj \
    .\inftrees.obj \
    .\infutil.obj
    @echo " Lib::Library Tool "
    ilib.exe  /nol Zlib.lib @makefile.rf;

.\uncompr.obj: \
    M:\Evgen\FreePM\utils\Zlib\uncompr.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h

.\trees.obj: \
    M:\Evgen\FreePM\utils\Zlib\trees.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}deflate.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}trees.h

.\infutil.obj: \
    M:\Evgen\FreePM\utils\Zlib\infutil.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infblock.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}inftrees.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infcodes.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infutil.h

.\inftrees.obj: \
    M:\Evgen\FreePM\utils\Zlib\inftrees.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}inftrees.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}inffixed.h

.\inflate.obj: \
    M:\Evgen\FreePM\utils\Zlib\inflate.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infblock.h

.\inffast.obj: \
    M:\Evgen\FreePM\utils\Zlib\inffast.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infblock.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}inftrees.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infcodes.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}inffast.h

.\infcodes.obj: \
    M:\Evgen\FreePM\utils\Zlib\infcodes.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infblock.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}inftrees.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infcodes.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}inffast.h

.\infblock.obj: \
    M:\Evgen\FreePM\utils\Zlib\infblock.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infblock.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}inftrees.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infcodes.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}infutil.h

.\gzio.obj: \
    M:\Evgen\FreePM\utils\Zlib\gzio.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h

.\deflate.obj: \
    M:\Evgen\FreePM\utils\Zlib\deflate.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}deflate.h

.\crc32.obj: \
    M:\Evgen\FreePM\utils\Zlib\crc32.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h

.\compress.obj: \
    M:\Evgen\FreePM\utils\Zlib\compress.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h

.\adler32.obj: \
    M:\Evgen\FreePM\utils\Zlib\adler32.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h

.\zutil.obj: \
    M:\Evgen\FreePM\utils\Zlib\zutil.c \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zutil.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\utils\Zlib;$(INCLUDE);}zconf.h
