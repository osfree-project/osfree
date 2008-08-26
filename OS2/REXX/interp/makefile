#
# A Makefile for Regina REXX Interpreter
# (c) osFree project,
# author, date
#

PROJ     = rexx
DESC     = Regina REXX interpreter
DEST     = os2

!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)regina.mk

$(PATH)rexx.exe: $(OBJS)

$(OBJS): $(HFILES)

$(PATH)drexx.obj: $(MYDIR)rexx.c $(HFILES)

$(PATH)client.obj: $(MYDIR)client.c $(HFILES) $(MYDIR)rexxsaa.h $(MYDIR)rxiface.h

$(PATH)rexxsaa.obj: $(MYDIR)rexxsaa.c $(HFILES) $(MYDIR)rexxsaa.h $(MYDIR)rxiface.h

$(PATH)yaccsrc.obj: $(MYDIR)yaccsrc.c $(MYDIR)defs.h $(MYDIR)rexx.h

$(PATH)lexsrc.obj: $(MYDIR)lexsrc.c $(MYDIR)yaccsrc.h $(MYDIR)rexx.h
