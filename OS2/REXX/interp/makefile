#
# A Makefile for Regina REXX Interpreter
# (c) osFree project,
# author, date
#

PROJ     = rexx
DESC     = Regina REXX interpreter

!include $(%ROOT)/mk/dirs.mk
!include $(MYDIR)regina.mk

#TARGETS  = $(PATH)rexx.exe # $(PATH)$(LIBFILE) $(PATH)regina.exe $(PATH)test1.dll &
#           $(PATH)test2.dll $(PATH)execiser.exe $(PATH)rxstack.exe # $(PATH)rxqueue.exe

HFILES = $(MYDIR)rexx.h $(MYDIR)defs.h $(MYDIR)extern.h $(MYDIR)regina_t.h $(MYDIR)configur.h $(MYDIR)strings.h

GCIHEADERS = $(MYDIR)gci\gci.h $(MYDIR)gci\embedded.h $(MYDIR)gci\gci_convert.h

$(PATH)rexx.exe: $(OBJS)

$(OBJS): $(HFILES)

$(PATH)drexx.obj: $(MYDIR)rexx.c $(HFILES)

$(PATH)client.obj: $(MYDIR)client.c $(HFILES) $(MYDIR)rexxsaa.h $(MYDIR)rxiface.h

$(PATH)rexxsaa.obj: $(MYDIR)rexxsaa.c $(HFILES) $(MYDIR)rexxsaa.h $(MYDIR)rxiface.h

$(PATH)yaccsrc.obj: $(PATH)yaccsrc.c $(MYDIR)defs.h $(MYDIR)rexx.h

$(PATH)lexsrc.obj: $(PATH)lexsrc.c $(PATH)yaccsrc.h $(MYDIR)rexx.h

s: .SYMBOLIC
 $(SAY) $(PATH)
 $(SAY) $(MYDIR)
 $(SAY) $(COPT)
