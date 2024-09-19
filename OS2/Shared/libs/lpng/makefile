#
# Makefile for common shared libs
#

PROJ = libpng
TRGT = $(PROJ).lib
ADD_COPT = -i=..$(SEP)include
TARGETS  = $(PATH)$(PROJ).lib

srcfiles = $(p)png$(e) &
$(p)pngerror$(e) &
$(p)pngget$(e) &
$(p)pngmem$(e) &
$(p)pngpread$(e) &
$(p)pngread$(e) &
$(p)pngrio$(e) &
$(p)pngrtran$(e) &
$(p)pngrutil$(e) &
$(p)pngset$(e) &
$(p)pngtrans$(e) &
$(p)pngwio$(e) &
$(p)pngwrite$(e) &
$(p)pngwtran$(e) &
$(p)pngwutil$(e)

!include $(%ROOT)tools/mk/libsos2.mk
!include $(MYDIR)libpng.mk

.c: $(PORT_BASE)$(SEP)lpng1644

.h: $(PORT_BASE)$(SEP)lpng1644

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
