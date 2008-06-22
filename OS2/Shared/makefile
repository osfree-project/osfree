# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..$(SEP)include
DIRS = libs

!include $(%ROOT)/mk/all.mk



all: libs $(ROOT)$(SEP)lib$(SEP)all_shared.lib msg .SYMBOLIC

OBJS = all_messages.obj all_querycurrentdisk.obj all_queryfsname.obj &
        all_dlist.obj all_pathmanipulation.obj all_performrecursiveaction.obj &
        all_getopt.obj

$(ROOT)$(SEP)lib$(SEP)all_shared.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @$(DC) $@
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)

libs: .symbolic
 @$(MAKE) $(MAKEOPT) TARGET=all subdirs

msg: .symbolic
 @$(if_not_exist_mkdir) msg $(LOG)
 @$(MC) @oso001.rsf $(LOG)
 @$(MC) @oso001h.rsf $(LOG)
 -@$(DC) $(FILESDIR)$(SEP)os2$(SEP)oso001.* $(LOG)
 -@$(DC) $(FILESDIR)$(SEP)os2$(SEP)oso001h.* $(LOG)
 -@$(CP) msg$(SEP)* $(FILESDIR)$(SEP)os2 $(LOG)
 -@rd msg $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
