# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#
# Makefile for Watcom/LIBC interface layer

32_BITS = 0
ADD_COPT  = -i=..$(SEP)include
srcfiles  = $(p)os2vdm$(e)

# For performance/compatibility reasons, we build a "_System" and
# "_Optlink"-like libraries, corresponding to "-5r" and "-5s" options of
# Watcom compiler.

TARGETS  = $(PATH)dos_shared.lib

!include $(%ROOT)/mk/libs.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
