# Makefile for common shared libs

PROJ = ojpeg
ADD_COPT = -i=..$(SEP)include

#CFLAGS         = $(CWARNS) -i.. -bd -bt=os2 -ze -zq -zp4 -onatxh -oe=120 -sg -ei -6r -fp6 -fpi87 -bm -mf /DNDEBUG /DOPJ_STATIC /DGBMMEM

srcfiles = $(p)bio$(e) $(p)cio$(e) $(p)dwt$(e) $(p)event$(e) $(p)image$(e) &
           $(p)j2k$(e) $(p)j2k_lib$(e) $(p)jp2$(e) $(p)jpt$(e) $(p)mct$(e) &
           $(p)mqc$(e) $(p)openjpeg$(e) $(p)pi$(e) $(p)raw$(e) $(p)t1$(e) &
           $(p)t2$(e) $(p)tcd$(e) $(p)tgt$(e)

!include $(%ROOT)/mk/libsos2.mk

TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
