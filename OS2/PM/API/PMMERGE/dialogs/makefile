# Makefile for PM library manager function

PROJ = win_dialog
ADD_COPT = -i=..$(SEP)include
srcfiles = $(p)windlgbox$(e)

!include $(%ROOT)/mk/libsos2.mk

TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
