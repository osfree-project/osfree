# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#

PROJ      = res
TRGT      = $(PROJ).lib
ADD_COPT  = -i=$(MYDIR)..$(SEP)..$(SEP)include
srcfiles  = $(p)res_parser$(e)
TARGETS  = $(PATH)$(TRGT)

!include $(%ROOT)tools/mk/libs.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
