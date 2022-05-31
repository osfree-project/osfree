#
# Makefile for DOS Commands Shared Library
#

PROJ      = doscmd_shared
TRGT      = $(PROJ).lib
ADD_COPT  = -i=$(MYDIR)..$(SEP)include
srcfiles  = $(p)tcc2wat$(e) $(p)kitten$(e)

TARGETS  = $(PATH)$(TRGT)

!include $(%ROOT)/mk/libsdos.mk

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library
