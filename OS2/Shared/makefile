# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..$(SEP)include
DIRS = libs

!include $(%ROOT)/mk/all.mk



all: $(ROOT)$(SEP)lib$(SEP)all_shared.lib msg .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

OBJS = all_messages.obj all_querycurrentdisk.obj all_queryfsname.obj &
        all_dlist.obj all_pathmanipulation.obj all_performrecursiveaction.obj &
        all_getopt.obj

$(ROOT)$(SEP)lib$(SEP)all_shared.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @$(DC) $@
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)

msg: .symbolic
 @$(if_not_exist_mkdir) msg $(LOG)
 @$(MC) @oso001.rsf $(LOG)
 @$(MC) @oso001h.rsf $(LOG)
 -@$(DC) $(ROOT)$(SEP)bin$(SEP)os2$(SEP)nls$(SEP)msg$(SEP)oso001.* $(LOG)
 -@$(DC) $(ROOT)$(SEP)bin$(SEP)os2$(SEP)nls$(SEP)msg$(SEP)oso001h.* $(LOG)
 -@$(CP) msg$(SEP)* $(ROOT)$(SEP)bin$(SEP)os2$(SEP)nls$(SEP)msg $(LOG)
 -@rd msg  $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
