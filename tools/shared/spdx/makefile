# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#

PROJ      = spdx
TRGT      = $(PROJ).lib
!ifeq %OS LINUX
defs = -d__LINUX__ -i=$(%WATCOM)$(SEP)lh
!else
defs = 
!endif
ADD_COPT  = -i=$(MYDIR)..$(SEP)..$(SEP)include $(defs)
srcfiles  = $(p)spdx_tag$(e) $(p)spdx_db$(e) $(p)spdx_discover$(e) $(p)spdx_utils$(e) $(p)spdx_lic$(e)
TARGETS  = $(PATH)$(TRGT)

!include $(%ROOT)tools/mk/libs.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
