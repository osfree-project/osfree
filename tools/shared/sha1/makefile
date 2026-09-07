# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#

PROJ      = sha1
TRGT      = $(PROJ).lib
ADD_COPT  = -i=$(MYDIR)..$(SEP)..$(SEP)include
srcfiles  = $(p)sha1_utils$(e)
TARGETS  = $(PATH)$(TRGT)

!include $(%ROOT)tools/mk/libs.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
