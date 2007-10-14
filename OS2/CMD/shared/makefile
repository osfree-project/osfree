# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#
# Makefile for Watcom/LIBC interface layer

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..$(SEP)include

!include $(%ROOT)/mk/all.mk

# For performance/compatibility reasons, we build a "_System" and
# "_Optlink"-like libraries, corresponding to "-5r" and "-5s" options of
# Watcom compiler.

all: $(ROOT)$(SEP)lib$(SEP)cmd_shared.lib msg

OBJS = cmd_ExecFSEntry.obj cmd_Messages.obj cmd_FSEntry16.obj &
       cmd_QueryCurrentDisk.obj cmd_QueryFSName.obj cmd_ShowVolumeInfo.obj

#cmd_fsentry16.obj: cmd_fsentry16.c
# $(SAY) Compiling $<
# @wcc -s -ml -zl -bt=os2 cmd_fsentry16.c $(LOG)

$(ROOT)$(SEP)lib$(SEP)cmd_shared.lib: $(OBJS)
 $(SAY) Linking $@
 -@if exist $@ @del $@ $(LOG)
 @wlib $@ +$<  $(LOG)

msg: .symbolic
 -@$(MKDIR) msg $(LOG)
 @$(MC) @lvm.rsf $(LOG)
 -@$(DC) $(ROOT)$(SEP)bin$(SEP)os2$(SEP)nls$(SEP)msg$(SEP)lvm.* $(LOG)
 -@$(CP) msg$(SEP)* $(ROOT)$(SEP)bin$(SEP)os2$(SEP)nls$(SEP)msg $(LOG)
# -@rd msg  $(LOG

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
