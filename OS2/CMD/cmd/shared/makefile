# $Id: makefile,v 1.3 2004/08/16 04:51:12 prokushev Exp $
#

PROJ = istring
srcfiles = $(p)istring$(e)
#UNI2H = 1
ADD_COPT = -i=$(%ROOT)include$(SEP)os3 -i=$(BLD)include &
     -i=$(MYDIR)..$(SEP)..$(SEP)include
DEST    = none

!include $(%ROOT)tools/mk/appsos2.mk
!include $(MYDIR)..$(SEP)port$(SEP)4os2.mk

ADD_COPT += -i=$(PORT_BASE)h
TARGETS = $(PATH)$(PROJ).$(O)
#TARGETS = $(OBJS)

$(PATH)$(PROJ).$(O):

.c: $(PORT_BASE)c

.h: $(PORT_BASE)h
