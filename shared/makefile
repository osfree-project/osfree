# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include ../build.conf
!include ../mk/site.mk

ROOT=..

32_BITS  = 1

ADD_COPT = -i=..\include
DIRS = libs

!include ../mk/all.mk



all: $(ROOT)\lib\all_shared.lib
 $(MAKE) TARGET=$^@ subdirs

OBJS = all_messages.obj all_querycurrentdisk.obj all_queryfsname.obj &
        all_dlist.obj all_pathmanipulation.obj all_performrecursiveaction.obj &
        all_getopt.obj

$(ROOT)\lib\all_shared.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @wlib $@ +$< $(LOG)

msg: .symbolic
 -@md msg $(LOG)
 @$(MC) @oso001.rsf $(LOG)
 @$(MC) @oso001h.rsf $(LOG)
 -@del $(ROOT)\bin\os2\nls\msg\oso001.* $(LOG)
 -@del $(ROOT)\bin\os2\nls\msg\oso001h.* $(LOG)
 -@move msg\* $(ROOT)\bin\os2\nls\msg $(LOG)
 -@rd msg  $(LOG)

# Clean all

clean:
 -@del $(LIBS) $(LOG)
 -@del *.obj $(LOG)
 -@del *.sbr $(LOG)
 -@del *.map $(LOG)
