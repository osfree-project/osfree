# makefile
# Created by IBM WorkFrame/2 MakeMake at 13:27:52 on 12 Nov 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Lib::Library Tool

.SUFFIXES:

.SUFFIXES: \
    .c .obj 

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\Zlib /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s

{M:\Evgen\FreePM\lib\JpegLib}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /I..\Zlib /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s

all: \
    .\JpegLib.lib

.\JpegLib.lib: \
    .\wrppm.obj \
    .\wrtarga.obj \
    .\jcapimin.obj \
    .\jcapistd.obj \
    .\jccoefct.obj \
    .\jccolor.obj \
    .\jcdctmgr.obj \
    .\jchuff.obj \
    .\jcinit.obj \
    .\jcmainct.obj \
    .\jcmarker.obj \
    .\jcmaster.obj \
    .\jcomapi.obj \
    .\jcparam.obj \
    .\jcphuff.obj \
    .\jcprepct.obj \
    .\jcsample.obj \
    .\jdapimin.obj \
    .\jdapistd.obj \
    .\jdatadst.obj \
    .\jdatasrc.obj \
    .\jdcoefct.obj \
    .\jdcolor.obj \
    .\jddctmgr.obj \
    .\jdhuff.obj \
    .\jdinput.obj \
    .\jdmainct.obj \
    .\jdmarker.obj \
    .\jdmaster.obj \
    .\jdmerge.obj \
    .\jdphuff.obj \
    .\jdpostct.obj \
    .\jdsample.obj \
    .\jerror.obj \
    .\jfdctflt.obj \
    .\jfdctfst.obj \
    .\jfdctint.obj \
    .\jidctflt.obj \
    .\jidctfst.obj \
    .\jidctint.obj \
    .\jidctred.obj \
    .\jmemmgr.obj \
    .\jmemnobs.obj \
    .\jquant1.obj \
    .\jquant2.obj \
    .\jutils.obj \
    .\rdbmp.obj \
    .\rdcolmap.obj \
    .\rdgif.obj \
    .\rdppm.obj \
    .\rdswitch.obj \
    .\rdtarga.obj \
    .\wrbmp.obj \
    .\wrgif.obj
    @echo " Lib::Library Tool "
    ilib.exe  /nol JpegLib.lib @makefile.rf;

.\wrppm.obj: \
    M:\Evgen\FreePM\lib\JpegLib\wrppm.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cdjpeg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cderror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\wrgif.obj: \
    M:\Evgen\FreePM\lib\JpegLib\wrgif.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cdjpeg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cderror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\wrbmp.obj: \
    M:\Evgen\FreePM\lib\JpegLib\wrbmp.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cdjpeg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cderror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\rdtarga.obj: \
    M:\Evgen\FreePM\lib\JpegLib\rdtarga.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cdjpeg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cderror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\rdswitch.obj: \
    M:\Evgen\FreePM\lib\JpegLib\rdswitch.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cdjpeg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cderror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\rdppm.obj: \
    M:\Evgen\FreePM\lib\JpegLib\rdppm.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cdjpeg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cderror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\rdgif.obj: \
    M:\Evgen\FreePM\lib\JpegLib\rdgif.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cdjpeg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cderror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\rdcolmap.obj: \
    M:\Evgen\FreePM\lib\JpegLib\rdcolmap.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cdjpeg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cderror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\rdbmp.obj: \
    M:\Evgen\FreePM\lib\JpegLib\rdbmp.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cdjpeg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cderror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jutils.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jutils.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jquant2.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jquant2.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jquant1.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jquant1.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jmemnobs.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jmemnobs.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmemsys.h

.\jmemmgr.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jmemmgr.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmemsys.h

.\jidctred.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jidctred.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jdct.h

.\jidctint.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jidctint.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jdct.h

.\jidctfst.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jidctfst.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jdct.h

.\jidctflt.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jidctflt.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jdct.h

.\jfdctint.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jfdctint.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jdct.h

.\jfdctfst.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jfdctfst.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jdct.h

.\jfdctflt.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jfdctflt.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jdct.h

.\jerror.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jerror.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jversion.h

.\jdsample.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdsample.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdpostct.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdpostct.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdphuff.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdphuff.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jdhuff.h

.\jdmerge.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdmerge.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdmaster.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdmaster.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdmarker.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdmarker.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdmainct.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdmainct.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdinput.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdinput.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdhuff.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdhuff.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jdhuff.h

.\jddctmgr.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jddctmgr.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jdct.h

.\jdcolor.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdcolor.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdcoefct.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdcoefct.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdatasrc.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdatasrc.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdatadst.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdatadst.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdapistd.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdapistd.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jdapimin.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jdapimin.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jcsample.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcsample.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jcprepct.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcprepct.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jcphuff.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcphuff.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jchuff.h

.\jcparam.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcparam.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jcomapi.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcomapi.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jcmaster.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcmaster.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jcmarker.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcmarker.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jcmainct.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcmainct.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jcinit.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcinit.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jchuff.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jchuff.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jchuff.h

.\jcdctmgr.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcdctmgr.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jdct.h

.\jccolor.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jccolor.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jccoefct.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jccoefct.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jcapistd.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcapistd.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\jcapimin.obj: \
    M:\Evgen\FreePM\lib\JpegLib\jcapimin.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h

.\wrtarga.obj: \
    M:\Evgen\FreePM\lib\JpegLib\wrtarga.c \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cdjpeg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jinclude.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpeglib.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jerror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}cderror.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jconfig.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jmorecfg.h \
    {M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}jpegint.h
