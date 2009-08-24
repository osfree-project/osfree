# Makefile for PM library manager function

PROJ = win_load
ADD_COPT = -i=..$(SEP)include
srcfiles = $(p)load$(e)

!include $(%ROOT)/mk/libsos2.mk

TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
