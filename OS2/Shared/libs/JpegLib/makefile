# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


PROJ = libjpeg
TRGT = $(PROJ).lib
ADD_COPT = -i=..$(SEP)include
TARGETS  = $(PATH)$(PROJ).lib

srcfiles = $(p)jaricom$(e) $(p)jcapimin$(e) $(p)jcapistd$(e) $(p)jcarith$(e) $(p)jccoefct$(e) $(p)jccolor$(e) &
        $(p)jcdctmgr$(e) $(p)jchuff$(e) $(p)jcinit$(e) $(p)jcmainct$(e) $(p)jcmarker$(e) $(p)jcmaster$(e) &
        $(p)jcomapi$(e) $(p)jcparam$(e) $(p)jcprepct$(e) $(p)jcsample$(e) $(p)jctrans$(e) $(p)jdapimin$(e) &
        $(p)jdapistd$(e) $(p)jdarith$(e) $(p)jdatadst$(e) $(p)jdatasrc$(e) $(p)jdcoefct$(e) $(p)jdcolor$(e) &
        $(p)jddctmgr$(e) $(p)jdhuff$(e) $(p)jdinput$(e) $(p)jdmainct$(e) $(p)jdmarker$(e) $(p)jdmaster$(e) &
        $(p)jdmerge$(e) $(p)jdpostct$(e) $(p)jdsample$(e) $(p)jdtrans$(e) $(p)jerror$(e) $(p)jfdctflt$(e) &
        $(p)jfdctfst$(e) $(p)jfdctint$(e) $(p)jidctflt$(e) $(p)jidctfst$(e) $(p)jidctint$(e) $(p)jquant1$(e) &
        $(p)jquant2$(e) $(p)jutils$(e) $(p)jmemmgr$(e)

!include $(%ROOT)tools/mk/libsos2.mk
!include $(MYDIR)ijg.mk

.c: $(PORT_BASE)jpeg-9e

.h: $(PORT_BASE)jpeg-9e

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
