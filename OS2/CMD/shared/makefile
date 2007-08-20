# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#
# Makefile for Watcom/LIBC interface layer

ROOT=..\..\..\

!include ../../../build.conf
!include ../../../mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..\include

!include ../../../mk/cmd.mk

# For performance/compatibility reasons, we build a "_System" and
# "_Optlink"-like libraries, corresponding to "-5r" and "-5s" options of
# Watcom compiler.

all: $(ROOT)\lib\cmd_shared.lib msg

OBJS = cmd_execfsentry.obj cmd_messages.obj cmd_fsentry16.obj &
       cmd_querycurrentdisk.obj cmd_queryfsname.obj cmd_showvolumeinfo.obj

#cmd_fsentry16.obj: cmd_fsentry16.c
# $(SAY) Compiling $<
# @wcc -s -ml -zl -bt=os2 cmd_fsentry16.c $(LOG)

$(ROOT)\lib\cmd_shared.lib: $(OBJS)
 $(SAY) Linking $@
 -@if exist $@ @del $@ $(LOG)
 @wlib $@ +$<  $(LOG)

msg: .symbolic
 -@md msg $(LOG)
 @$(MC) @lvm.rsf $(LOG)
 -@del $(ROOT)\bin\os2\nls\msg\lvm.* $(LOG)
 -@move msg\* $(ROOT)\bin\os2\nls\msg $(LOG)
 -@rd msg  $(LOG)

# Clean all

clean: .symbolic
 -@del $(LIBS) $(LOG)
 -@del *.obj $(LOG)
 -@del *.sbr $(LOG)
 -@del *.map $(LOG)
