###########################################################################
#
#	Makefile
#
#	Resource compiler makefile.
#
#	@(#)Makefile	2.17 4/15/97 15:52:54 /users/sccs/src/winsdk/rc/s.Makefile
#
#	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
#
#
#	TARGET is the destination of a build.
#	TWINDIR is the destination directory for any installations.
#
###########################################################################

include $(TWINDIR)/Makefile.config
include $(TWINDIR)/makeinclude/Makefile.version

TWIN_XFLAGS =  -DRC_VERSION=\"$(VERNAME)\"
TWIN_AUXINC =  -I$(TWINDIR)/win

# Installation command, used to control owner and privleges.
INSTALL_EXEC=install -m 0755


VERNAME = $(VERSION).$(PATCHLEVEL).$(ALPHA)

#
#
#

OBJFILES = lex.yy.o rc.y.tab.o rc.z.tab.o define.o resources.o util.o \
	   rc.o printres.o cmdline.o buffer.o cleanup.o mdc.o

RCNAME  = rc

all: $(RCNAME)

depend:

$(RCNAME): $(OBJFILES)
	$(CC) -o $(RCNAME) $(OBJFILES)

lex.yy.c: rclex.l Makefile
	$(LEX) -i rclex.l

rc.y.tab.c rc.y.tab.h: rcyacc.y Makefile
	$(YACC) -d rcyacc.y
	@mv y.tab.c rc.y.tab.c
	@mv y.tab.h rc.y.tab.h

rc.z.tab.c rc.z.tab.h: ifyacc.y Makefile
	$(YACC) -d ifyacc.y
	@sed -e "s/yy/zz/g" -e "s/YY/ZZ/g" y.tab.c >rc.z.tab.c
	@sed -e "s/yy/zz/g" -e "s/YY/ZZ/g" y.tab.h >rc.z.tab.h
	@rm y.tab.?

lex.yy.o: rc.y.tab.h rc.z.tab.h define.h rc.h
rc.y.tab.o: rc.h define.h buffer.h resfunc.h
rc.z.tab.o:
define.o: define.h
resources.o: rc.h buffer.h restypes.h resfunc.h
util.o: restypes.h
rc.o: rc.h define.h restypes.h
convert.o: rc.h restypes.h
nprint.o: rc.h restypes.h
buffer.o: rc.h buffer.h

#############################################################################
clean:
	rm -f $(RCNAME) $(OBJFILES) rc.y.* rc.z.* lex.yy.* *.o *.o *.o *.o *.o *.o *.o *.o *.a core *~ *.swp *.bak

clobber: clean
	rm -f rc.?.tab.? lex.yy.c

ctags:
	ctags *.c *.h >/dev/null 2>&1

sdiffs:
	@for f in $(SOURCE) $(HEADERS); do echo $$f;sdiffs $$f; done

install: $(RCNAME)
	strip $(RCNAME) ; $(INSTALL_EXEC) $(RCNAME) $(INSTALLDIR)/bin

#############################################################################
