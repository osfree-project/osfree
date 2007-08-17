# makefile
# Created by IBM WorkFrame/2 MakeMake at 13:27:52 on 12 Nov 2002
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Lib::Library Tool

!include ../../watcom.mif

#  $(sep)  -d2 
#sep=/
CC=wcc386
CPP=wcc386
CFLAGS= -i$(zlib_dir) -i. -bm -dHAVE_LIBJPEG -dHAVE_LIBPNG $(JPEGLIB_CFLAGS)
# -bm 

# '.obj' is changed to '$(OBJEXT)' 
# ' \' is changed to ' &'
# '.\' is changed to './'
# '{M:\Evgen\FreePM\lib\JpegLib}' is changed to '$(jpeg_dir)'
# '{M:\Evgen\FreePM\lib\JpegLib;..\Zlib;$(INCLUDE);}'   to  '$(jpeg_dir)'
# 'M:\Evgen\FreePM\lib\JpegLib\'    to    'jpeg_dir'


#jpeg_dir=$(sep)pub$(sep)FreePM$(sep)FreePM$(sep)lib$(sep)JpegLib


.SUFFIXES:

.SUFFIXES: &
     $(OBJEXT) .c

.c$(OBJEXT):
    @echo $[@
    #icc.exe /I..\Zlib /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s
    $(CPP) $(CFLAGS) -oaxt $[@

#jpeg_dir.c$(OBJEXT):
#   
#icc.exe /I..\Zlib /Ss /O /Gm /G5 /Gs /Gi /Oi32 /Gu /C %s

all: &
    .$(sep)JpegLib.lib

clean: 
	-rm *.o *.obj JpegLib.lib

.$(sep)JpegLib.lib: &
    .$(sep)wrppm$(OBJEXT) &
    .$(sep)wrtarga$(OBJEXT) &
    .$(sep)jcapimin$(OBJEXT) &
    .$(sep)jcapistd$(OBJEXT) &
    .$(sep)jccoefct$(OBJEXT) &
    .$(sep)jccolor$(OBJEXT) &
    .$(sep)jcdctmgr$(OBJEXT) &
    .$(sep)jchuff$(OBJEXT) &
    .$(sep)jcinit$(OBJEXT) &
    .$(sep)jcmainct$(OBJEXT) &
    .$(sep)jcmarker$(OBJEXT) &
    .$(sep)jcmaster$(OBJEXT) &
    .$(sep)jcomapi$(OBJEXT) &
    .$(sep)jcparam$(OBJEXT) &
    .$(sep)jcphuff$(OBJEXT) &
    .$(sep)jcprepct$(OBJEXT) &
    .$(sep)jcsample$(OBJEXT) &
    .$(sep)jdapimin$(OBJEXT) &
    .$(sep)jdapistd$(OBJEXT) &
    .$(sep)jdatadst$(OBJEXT) &
    .$(sep)jdatasrc$(OBJEXT) &
    .$(sep)jdcoefct$(OBJEXT) &
    .$(sep)jdcolor$(OBJEXT) &
    .$(sep)jddctmgr$(OBJEXT) &
    .$(sep)jdhuff$(OBJEXT) &
    .$(sep)jdinput$(OBJEXT) &
    .$(sep)jdmainct$(OBJEXT) &
    .$(sep)jdmarker$(OBJEXT) &
    .$(sep)jdmaster$(OBJEXT) &
    .$(sep)jdmerge$(OBJEXT) &
    .$(sep)jdphuff$(OBJEXT) &
    .$(sep)jdpostct$(OBJEXT) &
    .$(sep)jdsample$(OBJEXT) &
    .$(sep)jerror$(OBJEXT) &
    .$(sep)jfdctflt$(OBJEXT) &
    .$(sep)jfdctfst$(OBJEXT) &
    .$(sep)jfdctint$(OBJEXT) &
    .$(sep)jidctflt$(OBJEXT) &
    .$(sep)jidctfst$(OBJEXT) &
    .$(sep)jidctint$(OBJEXT) &
    .$(sep)jidctred$(OBJEXT) &
    .$(sep)jmemmgr$(OBJEXT) &
    .$(sep)jmemnobs$(OBJEXT) &
    .$(sep)jquant1$(OBJEXT) &
    .$(sep)jquant2$(OBJEXT) &
    .$(sep)jutils$(OBJEXT) &
    .$(sep)rdbmp$(OBJEXT) &
    .$(sep)rdcolmap$(OBJEXT) &
    .$(sep)rdgif$(OBJEXT) &
    .$(sep)rdppm$(OBJEXT) &
    .$(sep)rdswitch$(OBJEXT) &
    .$(sep)rdtarga$(OBJEXT) &
    .$(sep)wrbmp$(OBJEXT) &
    .$(sep)wrgif$(OBJEXT)
    @echo " Lib::Library Tool "
    #ilib.exe  /nol JpegLib.lib @makefile.rf;
    wlib JpegLib.lib $<

.$(sep)wrppm$(OBJEXT): &
    $(jpeg_dir)$(sep)wrppm.c &
    $(jpeg_dir)$(sep)cdjpeg.h &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)cderror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)wrgif$(OBJEXT): &
    $(jpeg_dir)$(sep)wrgif.c &
    $(jpeg_dir)$(sep)cdjpeg.h &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)cderror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)wrbmp$(OBJEXT): &
    $(jpeg_dir)$(sep)wrbmp.c &
    $(jpeg_dir)$(sep)cdjpeg.h &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)cderror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)rdtarga$(OBJEXT): &
    $(jpeg_dir)$(sep)rdtarga.c &
    $(jpeg_dir)$(sep)cdjpeg.h &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)cderror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)rdswitch$(OBJEXT): &
    $(jpeg_dir)$(sep)rdswitch.c &
    $(jpeg_dir)$(sep)cdjpeg.h &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)cderror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)rdppm$(OBJEXT): &
    $(jpeg_dir)$(sep)rdppm.c &
    $(jpeg_dir)$(sep)cdjpeg.h &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)cderror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)rdgif$(OBJEXT): &
    $(jpeg_dir)$(sep)rdgif.c &
    $(jpeg_dir)$(sep)cdjpeg.h &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)cderror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)rdcolmap$(OBJEXT): &
    $(jpeg_dir)$(sep)rdcolmap.c &
    $(jpeg_dir)$(sep)cdjpeg.h &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)cderror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)rdbmp$(OBJEXT): &
    $(jpeg_dir)$(sep)rdbmp.c &
    $(jpeg_dir)$(sep)cdjpeg.h &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)cderror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jutils$(OBJEXT): &
    $(jpeg_dir)$(sep)jutils.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jquant2$(OBJEXT): &
    $(jpeg_dir)$(sep)jquant2.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jquant1$(OBJEXT): &
    $(jpeg_dir)$(sep)jquant1.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jmemnobs$(OBJEXT): &
    $(jpeg_dir)$(sep)jmemnobs.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jmemsys.h

.$(sep)jmemmgr$(OBJEXT): &
    $(jpeg_dir)$(sep)jmemmgr.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jmemsys.h

.$(sep)jidctred$(OBJEXT): &
    $(jpeg_dir)$(sep)jidctred.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jdct.h

.$(sep)jidctint$(OBJEXT): &
    $(jpeg_dir)$(sep)jidctint.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jdct.h

.$(sep)jidctfst$(OBJEXT): &
    $(jpeg_dir)$(sep)jidctfst.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jdct.h

.$(sep)jidctflt$(OBJEXT): &
    $(jpeg_dir)$(sep)jidctflt.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jdct.h

.$(sep)jfdctint$(OBJEXT): &
    $(jpeg_dir)$(sep)jfdctint.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jdct.h

.$(sep)jfdctfst$(OBJEXT): &
    $(jpeg_dir)$(sep)jfdctfst.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jdct.h

.$(sep)jfdctflt$(OBJEXT): &
    $(jpeg_dir)$(sep)jfdctflt.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jdct.h

.$(sep)jerror$(OBJEXT): &
    $(jpeg_dir)$(sep)jerror.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jversion.h

.$(sep)jdsample$(OBJEXT): &
    $(jpeg_dir)$(sep)jdsample.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdpostct$(OBJEXT): &
    $(jpeg_dir)$(sep)jdpostct.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdphuff$(OBJEXT): &
    $(jpeg_dir)$(sep)jdphuff.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jdhuff.h

.$(sep)jdmerge$(OBJEXT): &
    $(jpeg_dir)$(sep)jdmerge.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdmaster$(OBJEXT): &
    $(jpeg_dir)$(sep)jdmaster.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdmarker$(OBJEXT): &
    $(jpeg_dir)$(sep)jdmarker.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdmainct$(OBJEXT): &
    $(jpeg_dir)$(sep)jdmainct.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdinput$(OBJEXT): &
    $(jpeg_dir)$(sep)jdinput.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdhuff$(OBJEXT): &
    $(jpeg_dir)$(sep)jdhuff.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jdhuff.h

.$(sep)jddctmgr$(OBJEXT): &
    $(jpeg_dir)$(sep)jddctmgr.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jdct.h

.$(sep)jdcolor$(OBJEXT): &
    $(jpeg_dir)$(sep)jdcolor.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdcoefct$(OBJEXT): &
    $(jpeg_dir)$(sep)jdcoefct.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdatasrc$(OBJEXT): &
    $(jpeg_dir)$(sep)jdatasrc.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdatadst$(OBJEXT): &
    $(jpeg_dir)$(sep)jdatadst.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdapistd$(OBJEXT): &
    $(jpeg_dir)$(sep)jdapistd.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jdapimin$(OBJEXT): &
    $(jpeg_dir)$(sep)jdapimin.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jcsample$(OBJEXT): &
    $(jpeg_dir)$(sep)jcsample.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jcprepct$(OBJEXT): &
    $(jpeg_dir)$(sep)jcprepct.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jcphuff$(OBJEXT): &
    $(jpeg_dir)$(sep)jcphuff.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jchuff.h

.$(sep)jcparam$(OBJEXT): &
    $(jpeg_dir)$(sep)jcparam.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jcomapi$(OBJEXT): &
    $(jpeg_dir)$(sep)jcomapi.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jcmaster$(OBJEXT): &
    $(jpeg_dir)$(sep)jcmaster.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jcmarker$(OBJEXT): &
    $(jpeg_dir)$(sep)jcmarker.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jcmainct$(OBJEXT): &
    $(jpeg_dir)$(sep)jcmainct.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jcinit$(OBJEXT): &
    $(jpeg_dir)$(sep)jcinit.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jchuff$(OBJEXT): &
    $(jpeg_dir)$(sep)jchuff.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jchuff.h

.$(sep)jcdctmgr$(OBJEXT): &
    $(jpeg_dir)$(sep)jcdctmgr.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h &
    $(jpeg_dir)$(sep)jdct.h

.$(sep)jccolor$(OBJEXT): &
    $(jpeg_dir)$(sep)jccolor.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jccoefct$(OBJEXT): &
    $(jpeg_dir)$(sep)jccoefct.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jcapistd$(OBJEXT): &
    $(jpeg_dir)$(sep)jcapistd.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)jcapimin$(OBJEXT): &
    $(jpeg_dir)$(sep)jcapimin.c &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h

.$(sep)wrtarga$(OBJEXT): &
    $(jpeg_dir)$(sep)wrtarga.c &
    $(jpeg_dir)$(sep)cdjpeg.h &
    $(jpeg_dir)$(sep)jinclude.h &
    $(jpeg_dir)$(sep)jpeglib.h &
    $(jpeg_dir)$(sep)jerror.h &
    $(jpeg_dir)$(sep)cderror.h &
    $(jpeg_dir)$(sep)jconfig.h &
    $(jpeg_dir)$(sep)jmorecfg.h &
    $(jpeg_dir)$(sep)jpegint.h
