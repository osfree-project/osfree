# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


PROJ = libjpeg
TRGT = $(PROJ).lib
ADD_COPT = -i=..$(SEP)include -dHAVE_LIBJPEG -dHAVE_LIBPNG
TARGETS  = $(PATH)$(PROJ).lib

srcfiles = $(p)wrppm$(e) $(p)wrtarga$(e) $(p)jcapimin$(e) $(p)jcapistd$(e) $(p)jccoefct$(e) $(p)jccolor$(e) &
           $(p)jcdctmgr$(e) $(p)jchuff$(e) $(p)jcinit$(e) $(p)jcmainct$(e) $(p)jcmarker$(e) $(p)jcmaster$(e) &
           $(p)jcomapi$(e) $(p)jcparam$(e) $(p)jcphuff$(e) $(p)jcprepct$(e) $(p)jcsample$(e) $(p)jdapimin$(e) &
           $(p)jdapistd$(e) $(p)jdatadst$(e) $(p)jdatasrc$(e) $(p)jdcoefct$(e) $(p)jdcolor$(e) $(p)jddctmgr$(e) &
           $(p)jdhuff$(e) $(p)jdinput$(e) $(p)jdmainct$(e) $(p)jdmarker$(e) $(p)jdmaster$(e) $(p)jdmerge$(e) &
           $(p)jdphuff$(e) $(p)jdpostct$(e) $(p)jdsample$(e) $(p)jerror$(e) $(p)jfdctflt$(e) $(p)jfdctfst$(e) &
           $(p)jfdctint$(e) $(p)jidctflt$(e) $(p)jidctfst$(e) $(p)jidctint$(e) $(p)jidctred$(e) $(p)jmemmgr$(e) &
           $(p)jmemnobs$(e) $(p)jquant1$(e) $(p)jquant2$(e) $(p)jutils$(e) $(p)rdbmp$(e) $(p)rdcolmap$(e) $(p)rdgif$(e) &
           $(p)rdppm$(e) $(p)rdswitch$(e) $(p)rdtarga$(e) $(p)wrbmp$(e) $(p)wrgif$(e)

!include $(%ROOT)/mk/libsos2.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
