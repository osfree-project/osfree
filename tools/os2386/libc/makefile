#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = clibext
TRGT = $(PROJ).lib
srcfiles = $(p)cgetsos2$(e) $(p)getchos2$(e) $(p)putchos2$(e) $(p)kbhitos2$(e)
# defines additional options for C compiler
ADD_COPT    =
NODEFAULTLIBS = 1
ADD_LINKOPT = lib sub32.lib
UNI2H = 1

#TARGETS  = $(PATH)$(PROJ).lib

!include $(%ROOT)/mk/libsos2.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
