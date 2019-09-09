# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#
# Makefile for Watcom/LIBC interface layer

PROJ      = dos_shared
TRGT      = $(PROJ).lib
#32_BITS = 0
ADD_COPT  = -i=$(MYDIR)..$(SEP)include
srcfiles  = $(p)os2vdm$(e)

TARGETS  = $(PATH)$(TRGT)

!include $(%ROOT)/mk/libsdos.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
