#########################################################################
#	@(#)Makefile	1.6
#
#	Makefile for building readline library for built-in debugger
#
#	Copyright 1995, Willows Software, Inc.
#
#########################################################################

include $(TWINDIR)/Makefile.config

#########################################################################

TWIN_XFLAGS = -DANSI_ARROWS -DHAVE_TCGETATTR -DHIDE -DUSE_DIRENT -DSYS_UNIX
TWIN_AUXINC = -I$(TWINDIR)/win

#########################################################################

READLINEOBJ = \
	complete.o editline.o sysunix.o

READLINESRC = \
	complete.c editline.c sysunix.c

READLINEHDR = \
	unix.h

#########################################################################

all:	readline.o

readline.o: $(READLINEOBJ) $(TWINDIR)/win/readline.o
	$(LD) -r -o readline.o $(READLINEOBJ)
	cp readline.o  $(TWINDIR)/win/readline.o

$(TWINDIR)/win/readline.o:

clobber: 	
	rm -rf readline.o $(READLINEOBJ) dbg.tab.c dbg.tab.h lex.yy.c

clean:	clobber

sdiffs:
	@for f in $(READLINESRC) $(READLINEHDR); do echo $$f;sdiffs $$f; done

ctags:
	ctags *.c *.h >/dev/null 2>&1

depend: $(READLINESRC)
	@-makedepend -f Makedepend.$(TARGET) -I. $(TWIN_SYSINC) $(READLINESRC) 2>/dev/null

list:
	@for f in $(READLINEOBJ); do echo $$f;done

##########################################################################
#
# Source file dependencies are located in Makedepend.$(TARGET)
#
include Makedepend.$(TARGET)

##########################################################################
#
#
# DO NOT DELETE THIS LINE -- make depend depends on it.
