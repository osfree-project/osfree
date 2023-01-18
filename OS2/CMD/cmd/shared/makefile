# $Id: makefile,v 1.3 2004/08/16 04:51:12 prokushev Exp $
#

PROJ = istring
sources = $(p)istring$(e)
ADD_COPT = -i=$(MYDIR)..$(SEP)contrib$(SEP)h -i=$(MYDIR)..$(SEP)..$(SEP)include -i=$(BLD)include
#UNI2H = 1
DEST    = none

!include $(%ROOT)tools/mk/appsos2.mk

TARGETS = $(PATH)$(PROJ).$(O)
#TARGETS = $(OBJS)

$(PATH)$(PROJ).$(O):

.c: $(MYDIR)..$(SEP)contrib$(SEP)c

.h: $(MYDIR)..$(SEP)contrib$(SEP)h
