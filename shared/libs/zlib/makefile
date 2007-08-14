# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#
# Makefile for Watcom/LIBC interface layer

ROOT=..\..\..\..
#ADD_COPT = -i=..\include
!include $(ROOT)\makefile.inc

# For performance/compatibility reasons, we build a "_System" and
# "_Optlink"-like libraries, corresponding to "-5r" and "-5s" options of
# Watcom compiler.

all: $(ROOT)\lib\zlib_shared.lib

OBJS = cmd_execfsentry.obj cmd_messages.obj &
       cmd_querycurrentdisk.obj cmd_queryfsname.obj cmd_showvolumeinfo.obj

cmd_fsentry16.obj: cmd_fsentry16.c
 $(SAY) Compiling $<
 @wcc -s -ml -zl -bt=os2 cmd_fsentry16.c $(LOG)

$(ROOT)\lib\cmd_shared.lib: $(OBJS) cmd_FSEntry16.obj
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


ZLIB_OBJS = adler32.obj compress.obj &
            deflate.obj infblock.obj &
            infcodes.obj inffast.obj &
            inflate.obj inftrees.obj &
            infutil.obj trees.obj &
            uncompr.obj zutil.obj
