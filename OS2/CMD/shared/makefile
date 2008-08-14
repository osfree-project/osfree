# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#
# Makefile for Watcom/LIBC interface layer

PROJ = cmd_shared
ADD_COPT = -i=..$(SEP)include

# For performance/compatibility reasons, we build a "_System" and
# "_Optlink"-like libraries, corresponding to "-5r" and "-5s" options of
# Watcom compiler.
srcfiles = $(p)cmd_ExecFSEntry$(e) $(p)cmd_Messages$(e) $(p)cmd_FSEntry16$(e) &
           $(p)cmd_QueryCurrentDisk$(e) $(p)cmd_QueryFSName$(e) $(p)cmd_ShowVolumeInfo$(e)

TARGETS  = $(PATH)$(PROJ).lib $(PATH)lvm.msg $(PATH)lvm.pl $(PATH)lvm.ru

!include $(%ROOT)/mk/libsos2.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

#cmd_fsentry16.obj: cmd_fsentry16.c
# $(SAY) Compiling $<
# @wcc -s -ml -zl -bt=os2 cmd_fsentry16.c $(LOG)



DIRS    =
DEST    = os2$(SEP)system

!include $(%ROOT)/mk/appsos2.mk

TARGETS = $(PATH)oso001.msg  $(PATH)oso001.pl  $(PATH)oso001.ru &
          $(PATH)oso001h.msg $(PATH)oso001h.pl $(PATH)oso001h.ru

$(PATH)lvm.rsf: .SYMBOLIC
 @$(MAKE) $(MAKEOPT) T=$^. rsf
