##########################################################################
#	@(#)Makefile	1.14 
#
#	Makefile for building Linux built-in debugger	
#
#	Copyright 1995, Willows Software Inc.
#
##########################################################################

include $(TWINDIR)/Makefile.config

#########################################################################

TWIN_XFLAGS = -DUSE_READLINE
TWIN_AUXINC = -I$(TWINDIR)/win -I.

BISONFLAGS = -p TWIN_dbg_
BISON = bison $(BISONFLAGS)
  
#########################################################################

DEBUGGEROBJ = \
	break.o db_disasm.o dbg.tab.o hash.o info.o lex.twin.o function.o \
	alias.o help.o alloca.o

DEBUGGERSRC = \
	break.c db_disasm.c dbg.tab.c hash.c info.c lex.twin.c function.c \
	alias.c help.c alloca.c

DEBUGGERHDR = \
	db_disasm.h  regpos.h alias.h

#########################################################################

all:	debugger.o

debugger.o: $(DEBUGGEROBJ) $(TWINDIR)/win/debugger.o
	$(LD) -r -o debugger.o $(DEBUGGEROBJ)
	cp debugger.o $(TWINDIR)/win/debugger.o

$(TWINDIR)/win/debugger.o:

dbg.tab.c dbg.tab.h: dbg.y
	$(BISON) -b dbg -d dbg.y

lex.twin.c: debug.l
	$(LEX) -I debug.l
	sed -e 's/yy/TWIN_dbg_/g' < lex.yy.c > lex.twin.c

clobber: 	
	rm -rf debugger.o $(DEBUGGEROBJ) dbg.tab.c dbg.tab.h lex.yy.c *~

clean:	clobber

ctags:
	ctags *.c *.h >/dev/null 2>&1

depend: 
	@-makedepend -f Makedepend.$(TARGET) -I. $(TWIN_SYSINC) $(TWIN_WININC) $(DEBUGGERSRC) 2>/dev/null

list:
	@for f in $(DEBUGGEROBJ); do echo $$f;done

link:	clean
	for f in $(DEBUGGEROBJ); do echo ../$$TARGET/$$EXT/$$f; ln -s ../$$TARGET/$$EXT/$$f $$f ; done 

writable:
	@ls -lrt *.[ch] |grep "\-rw"

##########################################################################
#
# Source file dependencies are located in Makedepend.$(TARGET)
#
include Makedepend.$(TARGET)

##########################################################################
#
#
# DO NOT DELETE THIS LINE -- make depend depends on it.
