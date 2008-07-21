# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#
# Makefile for Watcom/LIBC interface layer

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

#32_BITS  = 1

ADD_COPT = -i=..$(SEP)include

!include $(%ROOT)/mk/all.mk

# For performance/compatibility reasons, we build a "_System" and
# "_Optlink"-like libraries, corresponding to "-5r" and "-5s" options of
# Watcom compiler.

all: $(ROOT)$(SEP)lib$(SEP)dos_shared.lib

OBJS = os2vdm.$(O)

$(ROOT)$(SEP)lib$(SEP)dos_shared.lib: $(OBJS)
 $(SAY) Linking $@
 -@if exist $@ @del $@ $(LOG)
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
