# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..\include -dHAVE_LIBJPEG -dHAVE_LIBPNG

!include $(%ROOT)/mk/all.mk



all: $(ROOT)\lib\jpeglib.lib .SYMBOLIC

OBJS = wrppm.$(O) &
    wrtarga.$(O) &
    jcapimin.$(O) &
    jcapistd.$(O) &
    jccoefct.$(O) &
    jccolor.$(O) &
    jcdctmgr.$(O) &
    jchuff.$(O) &
    jcinit.$(O) &
    jcmainct.$(O) &
    jcmarker.$(O) &
    jcmaster.$(O) &
    jcomapi.$(O) &
    jcparam.$(O) &
    jcphuff.$(O) &
    jcprepct.$(O) &
    jcsample.$(O) &
    jdapimin.$(O) &
    jdapistd.$(O) &
    jdatadst.$(O) &
    jdatasrc.$(O) &
    jdcoefct.$(O) &
    jdcolor.$(O) &
    jddctmgr.$(O) &
    jdhuff.$(O) &
    jdinput.$(O) &
    jdmainct.$(O) &
    jdmarker.$(O) &
    jdmaster.$(O) &
    jdmerge.$(O) &
    jdphuff.$(O) &
    jdpostct.$(O) &
    jdsample.$(O) &
    jerror.$(O) &
    jfdctflt.$(O) &
    jfdctfst.$(O) &
    jfdctint.$(O) &
    jidctflt.$(O) &
    jidctfst.$(O) &
    jidctint.$(O) &
    jidctred.$(O) &
    jmemmgr.$(O) &
    jmemnobs.$(O) &
    jquant1.$(O) &
    jquant2.$(O) &
    jutils.$(O) &
    rdbmp.$(O) &
    rdcolmap.$(O) &
    rdgif.$(O) &
    rdppm.$(O) &
    rdswitch.$(O) &
    rdtarga.$(O) &
    wrbmp.$(O) &
    wrgif.$(O)



$(ROOT)\lib\jpeglib.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @wlib $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
