# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = png
ADD_COPT = -i=..$(SEP)include
TARGETS  = $(PATH)$(PROJ).lib

srcfiles = $(p)pngwtran$(e) $(p)pngwutil$(e) $(p)png$(e) $(p)pngerror$(e) $(p)pnggccrd$(e) &
           $(p)pngget$(e) $(p)pngmem$(e) $(p)pngread$(e) $(p)pngrio$(e) $(p)pngrtran$(e) &
           $(p)pngrutil$(e) $(p)pngset$(e) $(p)pngtrans$(e) $(p)pngvcrd$(e) $(p)pngwio$(e) &
           $(p)pngwrite$(e)

!include $(%ROOT)/mk/libsos2.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
