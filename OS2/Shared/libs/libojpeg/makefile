# Makefile for common shared libs

PROJ = ojpeg
TRGT = $(PROJ).lib
ADD_COPT = -i=..$(SEP)include

srcfiles = $(p)bio$(e) $(p)cio$(e) $(p)dwt$(e) $(p)event$(e) $(p)image$(e) &
           $(p)j2k$(e) $(p)j2k_lib$(e) $(p)jp2$(e) $(p)jpt$(e) $(p)mct$(e) &
           $(p)mqc$(e) $(p)openjpeg$(e) $(p)pi$(e) $(p)raw$(e) $(p)t1$(e) &
           $(p)t2$(e) $(p)tcd$(e) $(p)tgt$(e)

!include $(%ROOT)tools/mk/libsos2.mk
# !include $(MYDIR)openjpeg.mk

# .c: $(PORT_BASE)src$(SEP)lib$(SEP)openjp2

# .h: $(PORT_BASE)src$(SEP)lib$(SEP)openjp2

TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
