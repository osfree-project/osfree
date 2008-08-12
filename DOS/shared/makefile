# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#
# Makefile for Watcom/LIBC interface layer

ADD_COPT  = -i=..$(SEP)include
srcfiles  = $(p)os2vdm$(e)

# For performance/compatibility reasons, we build a "_System" and
# "_Optlink"-like libraries, corresponding to "-5r" and "-5s" options of
# Watcom compiler.

TARGETS  = $(PATH)dos_shared.lib
DEST     = ..$(SEP)build$(SEP)lib

!include $(%ROOT)/mk/all.mk

$(PATH)dos_shared.lib: $(OBJS)
 $(SAY) Linking $@
 -@if exist $@ @del $@ $(LOG)
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)
