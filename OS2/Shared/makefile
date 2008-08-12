# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

DIRS = msg # libs
srcfiles = $(p)all_messages$(e) $(p)all_querycurrentdisk$(e) $(p)all_queryfsname$(e) &
           $(p)all_dlist$(e) $(p)all_pathmanipulation$(e) $(p)all_performrecursiveaction$(e) &
           $(p)all_getopt$(e)

!include $(%ROOT)/mk/os2app.mk

ADD_COPT = -i=..$(SEP)include
DEST     = ..$(SEP)lib
TARGETS  = $(PATH)all_shared.lib

$(PATH)all_shared.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @$(DC) $@
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)
