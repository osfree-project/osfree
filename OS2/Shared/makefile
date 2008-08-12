# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = all_shared
DIRS = # msg # libs
srcfiles = $(p)all_messages$(e) $(p)all_querycurrentdisk$(e) $(p)all_queryfsname$(e) &
           $(p)all_dlist$(e) $(p)all_pathmanipulation$(e) $(p)all_performrecursiveaction$(e) &
           $(p)all_getopt$(e)

#qw: .SYMBOLIC
# @$(MAKE) $(MAKEOPT) b

!include $(%ROOT)/mk/os2app.mk

ADD_COPT = -i=..$(SEP)include
DEST     = ..$(SEP)lib
TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) L=$(TARGETS) O=$(OBJS) libbb

a: .SYMBOLIC
 $(SAY) $(TARGETS)
 $(SAY) $(SUF)
