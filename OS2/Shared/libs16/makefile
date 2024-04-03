# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

DIRS = libs
PROJ = all_shared16
TRGT = $(PROJ).lib
srcfiles = $(p)all_messages$(e) $(p)all_querycurrentdisk$(e) $(p)all_queryfsname$(e) &
           $(p)all_dlist$(e) $(p)all_pathmanipulation$(e) $(p)all_performrecursiveaction$(e) &
           $(p)all_debug$(e)
UNI2H = 0
NOLIBS = 1
ADD_COPT =
32_BITS = 0

!include $(%ROOT)tools/mk/libsos2v1.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

.c: $(MYDIR)..
