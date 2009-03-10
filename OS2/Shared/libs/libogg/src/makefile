# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = libogg
DIRS=dll
ADD_COPT = $(defines) -i=$(MYDIR) -i=$(MYDIR)..$(SEP)include

srcfiles = $(p)framing$(e) $(p)bitwise$(e)

!include $(%ROOT)/mk/libsos2.mk

TARGETS  = $(PATH)$(PROJ).lib subdirs

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
