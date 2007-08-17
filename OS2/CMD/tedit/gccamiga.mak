# Generated automatically from Makefile.in by configure.
# Makefile for THE
#
# The variable 'srcdir' refers to the source-distribution, and can be set with
# the configure script by "--srcdir=DIR".
#

#SHELL		= /bin/sh
THIS		= Makefile



VER=31
VER_DOT=3.1
THE_DIR=THE-$(VER_DOT)

srcdir		= ..
prefix		= /usr/local
exec_prefix	= ${prefix}
libdir		= ${exec_prefix}/lib
bindir		= ${exec_prefix}/bin
includedir		= ${prefix}/include
mandir		= ${prefix}/man

docdir		= $(srcdir)/doc
contribdir		= $(srcdir)/contrib
thisdir		=/home/mark/THE-3.1/ncurses

INSTALL		= $(srcdir)/install-sh

HTML_EXT		= .html

CC		= gcc
CFLAGS		= -O3 -Wall -fomit-frame-pointer

INCDIR		= $(srcdir)
CPPFLAGS	= -I. -I$(INCDIR) -DAMIGA -DGCC -DTHE_HOME_DIRECTORY=\"$(prefix)/THE/\"

CCFLAGS		= -c $(CFLAGS) $(CPPFLAGS)

LINK		=  $(CC)
LDFLAGS		=  -O3 -lncurses -lregina #-lm -lcrypt -lfl

################################################################################

H0=	../defines.h $(srcdir)/regex.h
H1=	../defines.h $(srcdir)/the.h $(srcdir)/vars.h $(srcdir)/defines.h $(srcdir)/proto.h
H2=	../defines.h $(srcdir)/the.h $(srcdir)/vars.h $(srcdir)/command.h $(srcdir)/defines.h $(srcdir)/proto.h $(srcdir)/getch.h $(srcdir)/key.h
H3=	../defines.h $(srcdir)/getch.h $(srcdir)/vars.h $(srcdir)/defines.h $(srcdir)/proto.h
H4=	../defines.h $(srcdir)/the.h $(srcdir)/vars.h $(srcdir)/query.h $(srcdir)/defines.h $(srcdir)/proto.h
H5=	../defines.h $(srcdir)/the.h $(srcdir)/vars.h $(srcdir)/thematch.h
H6=	../defines.h $(srcdir)/the.h $(srcdir)/vars.h $(srcdir)/directry.h $(srcdir)/defines.h $(srcdir)/proto.h
H7=	../defines.h $(srcdir)/the.h $(srcdir)/vars.h $(srcdir)/therexx.h $(srcdir)/proto.h $(srcdir)/defines.h $(srcdir)/query.h
H8=	../defines.h $(srcdir)/the.h $(srcdir)/vars.h $(srcdir)/therexx.h $(srcdir)/defines.h

################################################################################
all:	the the.man THE_Help.txt html

the.man:	manext $(OVERVIEW) $(COMM) $(COMMSOS) $(COMMSET) $(QUERY) $(APPENDIX) $(GLOSSARY)
	-chmod u+w *
	-cp $(docdir)/* .
	./manext $(OVERVIEW) $(COMM) $(COMMSOS) $(COMMSET) $(QUERY) $(APPENDIX) $(GLOSSARY) > the.man

THE_Help.txt:	manext
	-chmod u+w *
	-cp $(docdir)/* .
	echo "                         THE Quick Reference" > THE_Help_part01
	echo "                             Version $(VER_DOT)" > THE_Help_part02
	cp THE_Help_head THE_Help_part03
	./manext -q $(COMM) > THE_Help_part04
	cp THE_Help_set THE_Help_part05
	./manext -q $(COMMSET) > THE_Help_part06
	cp THE_Help_sos THE_Help_part07
	./manext -q $(COMMSOS) > THE_Help_part08
	cp THE_Help_tail THE_Help_part09
	cat THE_Help_part?? > THE_Help.txt
	rm -f THE_Help_*

html:	manext
	-chmod u+w *
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) TOCSTART > index$(HTML_EXT)
	./manext $(OVERVIEW) > overview.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) OVERVIEW overview.man index$(HTML_EXT) > overview$(HTML_EXT)
	./manext $(COMM) > comm.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) COMM comm.man index$(HTML_EXT) > comm$(HTML_EXT)
	./manext $(COMMSET) > commset.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) COMMSET commset.man index$(HTML_EXT) > commset$(HTML_EXT)
	./manext $(COMMSOS) > commsos.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) COMMSOS commsos.man index$(HTML_EXT) > commsos$(HTML_EXT)
	./manext $(QUERY) > query.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) QUERY    query.man index$(HTML_EXT)    > query$(HTML_EXT)
	./manext $(GLOSSARY) > glossary.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) GLOSSARY glossary.man index$(HTML_EXT) > glossary$(HTML_EXT)
	./manext $(APPENDIX1) > app1.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) APPENDIX1 app1.man index$(HTML_EXT) > app1$(HTML_EXT)
	./manext $(APPENDIX2) > app2.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) APPENDIX2 app2.man index$(HTML_EXT) > app2$(HTML_EXT)
	./manext $(APPENDIX3) > app3.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) APPENDIX3 app3.man index$(HTML_EXT) > app3$(HTML_EXT)
	./manext $(APPENDIX4) > app4.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) APPENDIX4 app4.man index$(HTML_EXT) > app4$(HTML_EXT)
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) APPENDIX5 $(docdir)/app5.htm index$(HTML_EXT) > app5$(HTML_EXT)
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) APPENDIX6 $(docdir)/app6.htm index$(HTML_EXT) > app6$(HTML_EXT)
	cp $(srcdir)/HISTORY history.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) HISTORY history.man index$(HTML_EXT) > history$(HTML_EXT)
	cp $(srcdir)/THE_Help.txt quickref.man
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) QUICKREF quickref.man index$(HTML_EXT) > quickref$(HTML_EXT)
	rexx $(srcdir)/man2html.rex $(HTML_EXT) $(VER_DOT) TOCEND >> index$(HTML_EXT)

helpviewer:	manext docclean
	-chmod u+w *
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) TOCSTART 'junk' index$(HTML_EXT)
	./manext $(OVERVIEW) > overview.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) OVERVIEW overview.man index$(HTML_EXT)
	./manext $(COMM) > comm.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) COMM comm.man index$(HTML_EXT)
	./manext $(COMMSET) > commset.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) COMMSET commset.man index$(HTML_EXT)
	./manext $(COMMSOS) > commsos.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) COMMSOS commsos.man index$(HTML_EXT)
	./manext $(QUERY) > query.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) QUERY    query.man index$(HTML_EXT)
	./manext $(GLOSSARY) > glossary.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) GLOSSARY glossary.man index$(HTML_EXT)
	./manext $(APPENDIX1) > app1.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) APPENDIX1 app1.man index$(HTML_EXT)
	./manext $(APPENDIX2) > app2.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) APPENDIX2 app2.man index$(HTML_EXT)
	./manext $(APPENDIX3) > app3.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) APPENDIX3 app3.man index$(HTML_EXT)
	./manext $(APPENDIX4) > app4.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) APPENDIX4 app4.man index$(HTML_EXT)
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) APPENDIX5 $(docdir)/app5.htm index$(HTML_EXT)
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) APPENDIX6 $(docdir)/app6.htm index$(HTML_EXT)
	cp $(srcdir)/HISTORY history.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) HISTORY history.man index$(HTML_EXT)
	cp $(srcdir)/THE_Help.txt quickref.man
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) QUICKREF quickref.man index$(HTML_EXT)
	rexx $(srcdir)/man2hv.rex $(HTML_EXT) $(VER_DOT) TOCEND 'junk' index$(HTML_EXT)
	cp $(docdir)/*.gif misc

combined: html
	the -w 15000 -b -p $(docdir)/preparecomb.the index$(HTML_EXT) overview$(HTML_EXT) comm$(HTML_EXT) commset$(HTML_EXT) commsos$(HTML_EXT) query$(HTML_EXT) glossary$(HTML_EXT) app1$(HTML_EXT) app2$(HTML_EXT) app3$(HTML_EXT) app4$(HTML_EXT) app5$(HTML_EXT) app6$(HTML_EXT) history$(HTML_EXT) quickref$(HTML_EXT)
	cat index$(HTML_EXT).new overview$(HTML_EXT).new comm$(HTML_EXT).new commset$(HTML_EXT).new commsos$(HTML_EXT).new query$(HTML_EXT).new glossary$(HTML_EXT).new app1$(HTML_EXT).new app2$(HTML_EXT).new app3$(HTML_EXT).new app4$(HTML_EXT).new app5$(HTML_EXT).new app6$(HTML_EXT).new history$(HTML_EXT).new quickref$(HTML_EXT).new > THE-$(VER_DOT)$(HTML_EXT)

install: the the.man THE_Help.txt
	$(INSTALL) -m 755 -d $(bindir)
	$(INSTALL) -m 755 -d $(prefix)/THE
	$(INSTALL) -m 755 -c ./the $(bindir)/the
	$(INSTALL) -m 644 -c ./the.man $(prefix)/THE/the.man
	$(INSTALL) -m 644 -c ./THE_Help.txt $(prefix)/THE/THE_Help.txt
	$(INSTALL) -m 644 -c $(srcdir)/append.the   $(prefix)/THE/append.the
	$(INSTALL) -m 644 -c $(srcdir)/comm.the     $(prefix)/THE/comm.the
	$(INSTALL) -m 644 -c $(srcdir)/build.the    $(prefix)/THE/build.the
	$(INSTALL) -m 644 -c $(srcdir)/setbuild.the $(prefix)/THE/setbuild.the
	$(INSTALL) -m 644 -c $(srcdir)/uncomm.the   $(prefix)/THE/uncomm.the
	$(INSTALL) -m 644 -c $(srcdir)/total.the    $(prefix)/THE/total.the
	$(INSTALL) -m 644 -c $(srcdir)/match.the    $(prefix)/THE/match.the
	$(INSTALL) -m 644 -c $(srcdir)/rm.the       $(prefix)/THE/rm.the
	$(INSTALL) -m 644 -c $(srcdir)/words.the    $(prefix)/THE/words.the
	$(INSTALL) -m 644 -c $(srcdir)/l.the        $(prefix)/THE/l.the
	$(INSTALL) -m 644 -c $(srcdir)/compile.the  $(prefix)/THE/compile.the
	$(INSTALL) -m 644 -c $(srcdir)/spell.the    $(prefix)/THE/spell.the
	$(INSTALL) -m 644 -c $(srcdir)/demo.the     $(prefix)/THE/demo.the

clean:
	-rm -f *.o trace the manext *.man *$(HTML_EXT) *.new config.log config.cache config.status

distclean: clean
	-rm -f Makefile

docclean:
	-rm -fr *.man *$(HTML_EXT) *.gif *.new misc comm commset commsos

mostlyclean: clean

realclean: distclean

#------------------------------------------------------------------------

XTRA_OBJS =  

OBJS =      \
	box.o      \
	colour.o   \
	column.o   \
	comm1.o    \
	comm2.o    \
	comm3.o    \
	comm4.o    \
	comm5.o    \
	commset1.o \
	commset2.o \
	commsos.o  \
	commutil.o \
	cursor.o   \
	default.o  \
	directry.o \
	edit.o     \
	error.o    \
	execute.o  \
	file.o     \
	thematch.o \
	getch.o    \
	linked.o   \
	mouse.o    \
	memory.o   \
	nonansi.o  \
	parser.o   \
	prefix.o   \
	print.o    \
	query.o    \
	regex.o    \
	reserved.o \
	rexx.o     \
	scroll.o   \
	show.o     \
	sort.o     \
	target.o   \
	the.o      \
	mygetopt.o \
	util.o

the: $(OBJS) $(XTRA_OBJS)
	$(LINK) $(OBJS) $(XTRA_OBJS) -o the $(LDFLAGS)


box.o: $(srcdir)/box.c $(H1)
	$(CC) $(CCFLAGS) -o box.o $(srcdir)/box.c

colour.o: $(srcdir)/colour.c $(H1)
	$(CC) $(CCFLAGS) -o colour.o $(srcdir)/colour.c

column.o: $(srcdir)/column.c $(H1)
	$(CC) $(CCFLAGS) -o column.o $(srcdir)/column.c

comm1.o: $(srcdir)/comm1.c $(H1)
	$(CC) $(CCFLAGS) -o comm1.o $(srcdir)/comm1.c

comm2.o: $(srcdir)/comm2.c $(H1)
	$(CC) $(CCFLAGS) -o comm2.o $(srcdir)/comm2.c

comm3.o: $(srcdir)/comm3.c $(H1)
	$(CC) $(CCFLAGS) -o comm3.o $(srcdir)/comm3.c

comm4.o: $(srcdir)/comm4.c $(H1)
	$(CC) $(CCFLAGS) -o comm4.o $(srcdir)/comm4.c

comm5.o: $(srcdir)/comm5.c $(H1)
	$(CC) $(CCFLAGS) -o comm5.o $(srcdir)/comm5.c

commset1.o: $(srcdir)/commset1.c $(H1)
	$(CC) $(CCFLAGS) -o commset1.o $(srcdir)/commset1.c

commset2.o: $(srcdir)/commset2.c $(H1)
	$(CC) $(CCFLAGS) -o commset2.o $(srcdir)/commset2.c

commsos.o: $(srcdir)/commsos.c $(H1)
	$(CC) $(CCFLAGS) -o commsos.o $(srcdir)/commsos.c

commutil.o: $(srcdir)/commutil.c $(H2)
	$(CC) $(CCFLAGS) -o commutil.o $(srcdir)/commutil.c

cursor.o: $(srcdir)/cursor.c $(H1)
	$(CC) $(CCFLAGS) -o cursor.o $(srcdir)/cursor.c

default.o: $(srcdir)/default.c $(H1)
	$(CC) $(CCFLAGS) -o default.o $(srcdir)/default.c

directry.o: $(srcdir)/directry.c $(H6)
	$(CC) $(CCFLAGS) -o directry.o $(srcdir)/directry.c

edit.o: $(srcdir)/edit.c $(H1)
	$(CC) $(CCFLAGS) -o edit.o $(srcdir)/edit.c

error.o: $(srcdir)/error.c $(H1)
	$(CC) $(CCFLAGS) -o error.o $(srcdir)/error.c

execute.o: $(srcdir)/execute.c $(H1)
	$(CC) $(CCFLAGS) -o execute.o $(srcdir)/execute.c

file.o: $(srcdir)/file.c $(H6)
	$(CC) $(CCFLAGS) -o file.o $(srcdir)/file.c

thematch.o: $(srcdir)/thematch.c $(H5)
	$(CC) $(CCFLAGS) -o thematch.o $(srcdir)/thematch.c

getch.o: $(srcdir)/getch.c $(H3)
	$(CC) $(CCFLAGS) -o getch.o $(srcdir)/getch.c

mygetopt.o: $(srcdir)/mygetopt.c $(H1)
	$(CC) $(CCFLAGS) -o mygetopt.o $(srcdir)/mygetopt.c

linked.o: $(srcdir)/linked.c $(H1)
	$(CC) $(CCFLAGS) -o linked.o $(srcdir)/linked.c

mouse.o: $(srcdir)/mouse.c $(H1)
	$(CC) $(CCFLAGS) -o mouse.o $(srcdir)/mouse.c

memory.o: $(srcdir)/memory.c $(H1)
	$(CC) $(CCFLAGS) -o memory.o $(srcdir)/memory.c

nonansi.o: $(srcdir)/nonansi.c $(H1)
	$(CC) $(CCFLAGS) -o nonansi.o $(srcdir)/nonansi.c

parser.o: $(srcdir)/parser.c $(H1)
	$(CC) $(CCFLAGS) -o parser.o $(srcdir)/parser.c

os2eas.o: $(srcdir)/os2eas.c $(H1)
	$(CC) $(CCFLAGS) -o os2eas.o $(srcdir)/os2eas.c

prefix.o: $(srcdir)/prefix.c $(H1)
	$(CC) $(CCFLAGS) -o prefix.o $(srcdir)/prefix.c

print.o: $(srcdir)/print.c $(H1)
	$(CC) $(CCFLAGS) -o print.o $(srcdir)/print.c

query.o: $(srcdir)/query.c $(H4)
	$(CC) $(CCFLAGS) -o query.o $(srcdir)/query.c

regex.o: $(srcdir)/regex.c $(H0)
	$(CC) $(CCFLAGS) -o regex.o $(srcdir)/regex.c

reserved.o: $(srcdir)/reserved.c $(H1)
	$(CC) $(CCFLAGS) -o reserved.o $(srcdir)/reserved.c

rexx.o: $(srcdir)/rexx.c $(H7)
	$(CC) $(CCFLAGS) -o rexx.o $(srcdir)/rexx.c

scroll.o: $(srcdir)/scroll.c $(H1)
	$(CC) $(CCFLAGS) -o scroll.o $(srcdir)/scroll.c

show.o: $(srcdir)/show.c $(H1)
	$(CC) $(CCFLAGS) -o show.o $(srcdir)/show.c

sort.o: $(srcdir)/sort.c $(H1)
	$(CC) $(CCFLAGS) -o sort.o $(srcdir)/sort.c

target.o: $(srcdir)/target.c $(H1)
	$(CC) $(CCFLAGS) -o target.o $(srcdir)/target.c

the.o: $(srcdir)/the.c $(H1)
	$(CC) $(CCFLAGS) -o the.o $(srcdir)/the.c

trace.o: $(srcdir)/trace.c $(H1)
	$(CC) $(CCFLAGS) -o trace.o $(srcdir)/trace.c

util.o: $(srcdir)/util.c $(H1)
	$(CC) $(CCFLAGS) -o util.o $(srcdir)/util.c

alloca.o: $(srcdir)/alloca.c
	$(CC) $(CCFLAGS) -o alloca.o $(srcdir)/alloca.c


manext: manext.o
	$(LINK) manext.o -o manext

manext.o: $(srcdir)/manext.c $(srcdir)/the.h
	$(CC) $(CCFLAGS) -I$(srcdir) -o manext.o $(srcdir)/manext.c


zip:
	zip thesrc$(VER) README INSTALL FAQ TODO COPYING HISTORY THE_Help.txt
	zip thesrc$(VER) overview appendix.1 appendix.2 appendix.3 appendix.4 glossary README.OS2
	zip thesrc$(VER) box.c colour.c comm*.c cursor.c default.c directry.c
	zip thesrc$(VER) edit.c error.c scroll.c column.c execute.c print.c alloca.c
	zip thesrc$(VER) file.c thematch.c getch.c mygetopt.c linked.c mouse.c memory.c
	zip thesrc$(VER) nonansi.c os2eas.c prefix.c query.c reserved.c regex.c
	zip thesrc$(VER) rexx.c show.c sort.c target.c the.c trace.c util.c parser.c
	zip thesrc$(VER) command.h defines.h directry.h thematch.h getch.h vars.h
	zip thesrc$(VER) key.h query.h proto.h therexx.h the.h regex.h mygetopt.h
	zip thesrc$(VER) manext.c *.rsp *.def *.diz files.rcs the*.xbm *.ico the.res *.rc the.eas
	zip thesrc$(VER) append.the comm.the uncomm.the total.the match.the rm.the build.the setbuild.the
	zip thesrc$(VER) words.the l.the compile.the spell.the demo.the demo.txt
	zip thesrc$(VER) Makefile.in configure defines.h.in $(contribdir)/*.c
	zip thesrc$(VER) *.tld *.mak thedit.lnk $(docdir)/THE_Help* $(docdir)/*.gif $(docdir)/*.htm $(docdir)/*.the
	zip thesrc$(VER) config.guess config.sub install-sh
	zip thesrc$(VER) aclocal.m4 configure.in
	zip thesrc$(VER) man2html.rex man2hv.rex makedist.cmd tld2c.rex

zzip:
	(cd $(srcdir)/..; zip thesrc$(VER) $(ARCHIVE_FILES) )

dist:
	(cd $(srcdir)/..; tar cvf - \
	$(ARCHIVE_FILES) \
	| gzip > $(THE_DIR).tar.gz)

tar:
	(cd $(srcdir)/..; tar cvf - \
	$(ARCHIVE_FILES) \
	| compress > $(THE_DIR).tar.Z)
