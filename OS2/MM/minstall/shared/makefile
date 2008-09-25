# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#
# Makefile for Watcom/LIBC interface layer

PROJ = mm_shared
ADD_COPT = -i=$(MYDIR)..$(SEP)include

# For performance/compatibility reasons, we build a "_System" and
# "_Optlink"-like libraries, corresponding to "-5r" and "-5s" options of
# Watcom compiler.
srcfiles = $(p)file$(e) $(p)globstr$(e) &
           $(p)mciini$(e) $(p)msg$(e) $(p)dll$(e) $(p)crc$(e)

TARGETS  = $(PATH)$(PROJ).lib

!include $(%ROOT)/mk/libsos2.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

