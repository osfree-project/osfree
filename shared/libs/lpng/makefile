# makefile
# Created by IBM WorkFrame/2 MakeMake at 16:23:48 on 29 Sept 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES:

.SUFFIXES: \
    .c .obj 

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\Zlib /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s

{M:\Evgen\FreePM\lib\lpng}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\Zlib /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s

all: \
    .\example.exe

.\example.exe: \
    .\pngwtran.obj \
    .\pngwutil.obj \
    .\png.obj \
    .\pngerror.obj \
    .\pnggccrd.obj \
    .\pngget.obj \
    .\pngmem.obj \
    .\pngread.obj \
    .\pngrio.obj \
    .\pngrtran.obj \
    .\pngrutil.obj \
    .\pngset.obj \
    .\pngtest.obj \
    .\pngtrans.obj \
    .\pngvcrd.obj \
    .\pngwio.obj \
    .\pngwrite.obj \
    ..\Zlib\Zlib.LIB
    @echo " Link::Linker "
    icc.exe @<<
     /B" /exepack:2 /packd /optfunc /noe"
     /Feexample.exe 
     ..\Zlib\Zlib.LIB 
     .\pngwtran.obj
     .\pngwutil.obj
     .\png.obj
     .\pngerror.obj
     .\pnggccrd.obj
     .\pngget.obj
     .\pngmem.obj
     .\pngread.obj
     .\pngrio.obj
     .\pngrtran.obj
     .\pngrutil.obj
     .\pngset.obj
     .\pngtest.obj
     .\pngtrans.obj
     .\pngvcrd.obj
     .\pngwio.obj
     .\pngwrite.obj
<<

.\pngwtran.obj: \
    M:\Evgen\FreePM\lib\lpng\pngwtran.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngwrite.obj: \
    M:\Evgen\FreePM\lib\lpng\pngwrite.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngwio.obj: \
    M:\Evgen\FreePM\lib\lpng\pngwio.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngvcrd.obj: \
    M:\Evgen\FreePM\lib\lpng\pngvcrd.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngtrans.obj: \
    M:\Evgen\FreePM\lib\lpng\pngtrans.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngtest.obj: \
    M:\Evgen\FreePM\lib\lpng\pngtest.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngset.obj: \
    M:\Evgen\FreePM\lib\lpng\pngset.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngrutil.obj: \
    M:\Evgen\FreePM\lib\lpng\pngrutil.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngrtran.obj: \
    M:\Evgen\FreePM\lib\lpng\pngrtran.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngrio.obj: \
    M:\Evgen\FreePM\lib\lpng\pngrio.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngread.obj: \
    M:\Evgen\FreePM\lib\lpng\pngread.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngmem.obj: \
    M:\Evgen\FreePM\lib\lpng\pngmem.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngget.obj: \
    M:\Evgen\FreePM\lib\lpng\pngget.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pnggccrd.obj: \
    M:\Evgen\FreePM\lib\lpng\pnggccrd.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngerror.obj: \
    M:\Evgen\FreePM\lib\lpng\pngerror.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\png.obj: \
    M:\Evgen\FreePM\lib\lpng\png.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h

.\pngwutil.obj: \
    M:\Evgen\FreePM\lib\lpng\pngwutil.c \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}png.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zlib.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}pngconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}zconf.h \
    {M:\Evgen\FreePM\lib\lpng;..\Zlib;$(INCLUDE);}alloc.h
