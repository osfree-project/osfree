#
#########################################################################
#
# makefile for The Hessling Editor (THE)
# emx os/2
# If at some stage THE is built with the curses.dll and the regina.dll 
# then THE will need to be built with -Zomf -Zcrtdll -mprobe -lwrap -s
# and linked to the PDCurses and Regina import libraries:
# curses.lib and regina.lib
#
# You need the following environment variable set like:
# THE_SRCDIR=c:/the
# THE_SRCDIRN=c:\the
# PDCURSES_BINDIR=c:/pdc/emx
#
#########################################################################
#

VER = 31
VERDOT = 3.1

ifeq ($(AOUT),Y)
LIBEXT=a
OBJCFLAG=
OBJLDFLAG=
OBJLDFLAGOPT=
OBJTYPE=o
BIND=emxbind the
LXLITE=
else
LIBEXT=lib
OBJCFLAG=-Zomf
OBJLDFLAG=-Zomf
OBJLDFLAGOPT=-lwrap -s
OBJTYPE=obj
BIND=
LXLITE=lxlite the.exe
endif

SRC       = $(THE_SRCDIR)
ASRC      = $(THE_SRCDIRN)
CURSBIN   = $(PDCURSES_BINDIR)
CURSLIB   = $(PDCURSES_EMXDIR)/curses.$(LIBEXT)
CURSINC   = -I$(PDCURSES_SRCDIR)
REGINA_REXXLIBS = $(REGINA_EMXDIR)/regina.$(LIBEXT)
REGINA_REXXINC = -I$(REGINA_SRCDIR) -DUSE_REGINA
OS2REXX_REXXINC = -DUSE_OS2REXX
REXXTRANS_REXXLIBS = $(REXXTRANS_BINDIR)/rexxtrans.$(LIBEXT)
REXXTRANS_REXXINC = -I$(REXXTRANS_SRCDIR) -DUSE_REXXTRANS

docdir = $(ASRC)\doc
HTML_EXT = .html

COMM = \
	$(ASRC)\comm1.c \
	$(ASRC)\comm2.c \
	$(ASRC)\comm3.c \
	$(ASRC)\comm4.c \
	$(ASRC)\comm5.c

COMMSOS = \
	$(ASRC)\commsos.c

COMMSET = \
	$(ASRC)\commset1.c \
	$(ASRC)\commset2.c

QUERY = $(ASRC)\query.c

APPENDIX1 = $(ASRC)\appendix.1
APPENDIX2 = $(ASRC)\appendix.2
APPENDIX3 = $(ASRC)\appendix.3
APPENDIX4 = $(ASRC)\appendix.4
APPENDIX7 = $(ASRC)\appendix.7
APPENDIX = $(APPENDIX1) $(APPENDIX2) $(APPENDIX3) $(APPENDIX4) $(APPENDIX7)

GLOSSARY = $(ASRC)\glossary
OVERVIEW = $(ASRC)\overview

#########################################################################
# EMX compiler on OS/2
#########################################################################

ifeq ($(INT),OS2REXX)
REXXLIB = $(OS2REXX_REXXLIBS)
REXXINC =  $(OS2REXX_REXXINC)
endif
ifeq ($(INT),REXXTRANS)
REXXLIB = $(REXXTRANS_REXXLIBS)
REXXINC =  $(REXXTRANS_REXXINC)
endif
ifeq ($(INT),REGINA)
REXXLIB = $(REGINA_REXXLIBS)
REXXINC =  $(REGINA_REXXINC)
endif

ifeq ($(DEBUG),Y)
CFLAGS    = $(OBJCFLAG) -c -g -Wall -DTHE_TRACE -DNOVIO -DSTDC_HEADERS -DHAVE_PROTO -I$(SRC) $(CURSINC) $(REXXINC) -o$@
LDEBUG    = -g
TRACE     = trace.$(OBJTYPE)
LXLITE    =
else
CFLAGS    = $(OBJCFLAG) -c -O3 -fomit-frame-pointer -Wall -DNOVIO -DSTDC_HEADERS -DHAVE_PROTO -I$(SRC) $(CURSINC) $(REXXINC) -o$@
LDEBUG    = $(OBJLDFLAGOPT)
TRACE     =
endif

LDFLAGS   = $(OBJLDFLAG) $(LDEBUG)

CC        = gcc
LD        = $(CC) $(LDFLAGS) -o the *.$(OBJTYPE) $(CURSLIB) $(REXXLIB)
THERES    = $(ASRC)\the.res
RCTHE     = rc $(THERES) the.exe
#########################################################################
#
#
# Object files
#
OBJ1A = box.$(OBJTYPE) colour.$(OBJTYPE) comm1.$(OBJTYPE) comm2.$(OBJTYPE) comm3.$(OBJTYPE) comm4.$(OBJTYPE) comm5.$(OBJTYPE)
OBJ1B = commset1.$(OBJTYPE) commset2.$(OBJTYPE) commsos.$(OBJTYPE) cursor.$(OBJTYPE) default.$(OBJTYPE)
OBJ1C = edit.$(OBJTYPE) error.$(OBJTYPE) execute.$(OBJTYPE) linked.$(OBJTYPE) column.$(OBJTYPE) mouse.$(OBJTYPE)
OBJ1D = nonansi.$(OBJTYPE) prefix.$(OBJTYPE) reserved.$(OBJTYPE) scroll.$(OBJTYPE) show.$(OBJTYPE) sort.$(OBJTYPE)
OBJ1E = memory.$(OBJTYPE) target.$(OBJTYPE) the.$(OBJTYPE) util.$(OBJTYPE) parser.$(OBJTYPE)
OBJ1 = $(OBJ1A) $(OBJ1B) $(OBJ1C) $(OBJ1D) $(OBJ1E)
OBJ2 = commutil.$(OBJTYPE) print.$(OBJTYPE) $(TRACE)
OBJ3 = getch.$(OBJTYPE)
OBJ4 = query.$(OBJTYPE)
OBJ5 = thematch.$(OBJTYPE) regex.$(OBJTYPE) mygetopt.$(OBJTYPE)
OBJ6 = directry.$(OBJTYPE) file.$(OBJTYPE)
OBJ7 = rexx.$(OBJTYPE)
OBJ8 = os2eas.$(OBJTYPE)
OBJOTH = $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8)
OBJS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8)

COMM = $(SRC)/comm1.c $(SRC)/comm2.c $(SRC)/comm3.c $(SRC)/comm4.c $(SRC)/comm5.c \
	$(SRC)/commsos.c $(SRC)/commset1.c $(SRC)/commset2.c $(SRC)/query.c

all: the.exe dist
#
#########################################################################
the.exe:	$(OBJS)
	@echo $(OBJ1A) > the.rsp
	@echo $(OBJ1B) >> the.rsp
	@echo $(OBJ1C) >> the.rsp
	@echo $(OBJ1D) >> the.rsp
	@echo $(OBJ1E) >> the.rsp
	@echo $(OBJOTH) >> the.rsp
	$(LD)
	$(BIND)
	del the.rsp
	$(RCTHE)
	$(LXLITE)
#########################################################################
box.$(OBJTYPE):	$(SRC)/box.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/box.c
colour.$(OBJTYPE):	$(SRC)/colour.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/colour.c
comm1.$(OBJTYPE):	$(SRC)/comm1.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/comm1.c
comm2.$(OBJTYPE):	$(SRC)/comm2.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/comm2.c
comm3.$(OBJTYPE):	$(SRC)/comm3.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/comm3.c
comm4.$(OBJTYPE):	$(SRC)/comm4.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/comm4.c
comm5.$(OBJTYPE):	$(SRC)/comm5.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/comm5.c
commset1.$(OBJTYPE):	$(SRC)/commset1.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/commset1.c
commset2.$(OBJTYPE):	$(SRC)/commset2.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/commset2.c
commsos.$(OBJTYPE):	$(SRC)/commsos.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/commsos.c
cursor.$(OBJTYPE):	$(SRC)/cursor.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/cursor.c
default.$(OBJTYPE):	$(SRC)/default.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/default.c
edit.$(OBJTYPE):	$(SRC)/edit.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/edit.c
error.$(OBJTYPE):	$(SRC)/error.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/error.c
execute.$(OBJTYPE):	$(SRC)/execute.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/execute.c
linked.$(OBJTYPE):	$(SRC)/linked.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/linked.c
column.$(OBJTYPE):	$(SRC)/column.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/column.c
mouse.$(OBJTYPE):	$(SRC)/mouse.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/mouse.c
memory.$(OBJTYPE):	$(SRC)/memory.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/memory.c
nonansi.$(OBJTYPE):	$(SRC)/nonansi.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/nonansi.c
os2eas.$(OBJTYPE):	$(SRC)/os2eas.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/os2eas.c
prefix.$(OBJTYPE):	$(SRC)/prefix.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/prefix.c
print.$(OBJTYPE):	$(SRC)/print.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/print.c
reserved.$(OBJTYPE):	$(SRC)/reserved.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/reserved.c
scroll.$(OBJTYPE):	$(SRC)/scroll.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/scroll.c
show.$(OBJTYPE):	$(SRC)/show.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/show.c
sort.$(OBJTYPE):	$(SRC)/sort.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/sort.c
target.$(OBJTYPE):	$(SRC)/target.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/target.c
the.$(OBJTYPE):	$(SRC)/the.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/the.c
util.$(OBJTYPE):	$(SRC)/util.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/util.c
commutil.$(OBJTYPE):	$(SRC)/commutil.c $(SRC)/the.h $(SRC)/command.h $(SRC)/defines.h $(SRC)/proto.h $(SRC)/getch.h $(SRC)/key.h
	$(CC) $(CFLAGS) $(SRC)/commutil.c
trace.$(OBJTYPE):	$(SRC)/trace.c $(SRC)/the.h $(SRC)/command.h $(SRC)/defines.h $(SRC)/proto.h $(SRC)/getch.h $(SRC)/key.h
	$(CC) $(CFLAGS) $(SRC)/trace.c
getch.$(OBJTYPE):	$(SRC)/getch.c $(SRC)/getch.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/getch.c
query.$(OBJTYPE):	$(SRC)/query.c $(SRC)/query.h $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/query.c
thematch.$(OBJTYPE):	$(SRC)/thematch.c $(SRC)/the.h $(SRC)/thematch.h
	$(CC) $(CFLAGS) $(SRC)/thematch.c
parser.$(OBJTYPE):	$(SRC)/parser.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/parser.c
directry.$(OBJTYPE):	$(SRC)/directry.c $(SRC)/the.h $(SRC)/directry.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/directry.c
file.$(OBJTYPE):	$(SRC)/file.c $(SRC)/the.h $(SRC)/directry.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/file.c
rexx.$(OBJTYPE):	$(SRC)/rexx.c $(SRC)/the.h $(SRC)/therexx.h $(SRC)/proto.h $(SRC)/defines.h $(SRC)/query.h
	$(CC) $(CFLAGS) $(SRC)/rexx.c
regex.$(OBJTYPE):	$(SRC)/regex.c $(SRC)/regex.h
	$(CC) $(CFLAGS) $(SRC)/regex.c
mygetopt.$(OBJTYPE):	$(SRC)/mygetopt.c $(SRC)/mygetopt.h
	$(CC) $(CFLAGS) $(SRC)/mygetopt.c
########################################################################
dist: the.exe
	echo run $(ASRC)\makedist.cmd
#########################################################################

manext.exe: manext.o
	$(CC) manext.o -o manext
	emxbind manext

manext.o: $(SRC)\manext.c $(SRC)\the.h
	$(CC) -c -I$(SRC) -o manext.o $(SRC)/manext.c

html:	manext.exe
	-del *$(HTML_EXT)
	copy $(ASRC)\man2html.rex .\man2html.cmd
	man2html $(HTML_EXT) $(VERDOT) TOCSTART > index$(HTML_EXT)
	.\manext $(OVERVIEW) > overview.man
	man2html $(HTML_EXT) $(VERDOT) OVERVIEW overview.man index$(HTML_EXT) > overview$(HTML_EXT)
	.\manext $(COMM) > comm.man
	man2html $(HTML_EXT) $(VERDOT) COMM comm.man index$(HTML_EXT) > comm$(HTML_EXT)
	.\manext $(COMMSET) > commset.man
	man2html $(HTML_EXT) $(VERDOT) COMMSET commset.man index$(HTML_EXT) > commset$(HTML_EXT)
	.\manext $(COMMSOS) > commsos.man
	man2html $(HTML_EXT) $(VERDOT) COMMSOS commsos.man index$(HTML_EXT) > commsos$(HTML_EXT)
	.\manext $(QUERY) > query.man
	man2html $(HTML_EXT) $(VERDOT) QUERY    query.man index$(HTML_EXT)    > query$(HTML_EXT)
	.\manext $(GLOSSARY) > glossary.man
	man2html $(HTML_EXT) $(VERDOT) GLOSSARY glossary.man index$(HTML_EXT) > glossary$(HTML_EXT)
	.\manext $(APPENDIX1) > app1.man
	man2html $(HTML_EXT) $(VERDOT) APPENDIX1 app1.man index$(HTML_EXT) > app1$(HTML_EXT)
	.\manext $(APPENDIX2) > app2.man
	man2html $(HTML_EXT) $(VERDOT) APPENDIX2 app2.man index$(HTML_EXT) > app2$(HTML_EXT)
	.\manext $(APPENDIX3) > app3.man
	man2html $(HTML_EXT) $(VERDOT) APPENDIX3 app3.man index$(HTML_EXT) > app3$(HTML_EXT)
	.\manext $(APPENDIX4) > app4.man
	man2html $(HTML_EXT) $(VERDOT) APPENDIX4 app4.man index$(HTML_EXT) > app4$(HTML_EXT)
	man2html $(HTML_EXT) $(VERDOT) APPENDIX5 $(docdir)/app5.htm index$(HTML_EXT) > app5$(HTML_EXT)
	man2html $(HTML_EXT) $(VERDOT) APPENDIX6 $(docdir)/app6.htm index$(HTML_EXT) > app6$(HTML_EXT)
	.\manext $(APPENDIX7) > app7.man
	man2html $(HTML_EXT) $(VERDOT) APPENDIX7 app7.man index$(HTML_EXT) > app7$(HTML_EXT)
	copy $(ASRC)\HISTORY history.man
	man2html $(HTML_EXT) $(VERDOT) HISTORY history.man index$(HTML_EXT) > history$(HTML_EXT)
	copy $(ASRC)\THE_Help.txt quickref.man
	man2html $(HTML_EXT) $(VERDOT) QUICKREF quickref.man index$(HTML_EXT) > quickref$(HTML_EXT)
	man2html $(HTML_EXT) $(VERDOT) TOCEND >> index$(HTML_EXT)

helpviewer:	manext.exe
	-del /Y *$(HTML_EXT)
	-del /Y *.man
	-del /Y comm\\*.*
	-del /Y commsos\\*.*
	-del /Y commset\\*.*
	-del /Y misc\\*.*
	-mkdir comm
	-mkdir commsos
	-mkdir commset
	-mkdir misc
	copy $(ASRC)\man2hv.rex .\man2hv.cmd
	man2hv $(HTML_EXT) $(VERDOT) TOCSTART 'junk' index$(HTML_EXT)
	.\manext $(OVERVIEW) > overview.man
	man2hv $(HTML_EXT) $(VERDOT) OVERVIEW overview.man index$(HTML_EXT)
	.\manext $(COMM) > comm.man
	man2hv $(HTML_EXT) $(VERDOT) COMM comm.man index$(HTML_EXT)
	.\manext $(COMMSET) > commset.man
	man2hv $(HTML_EXT) $(VERDOT) COMMSET commset.man index$(HTML_EXT)
	.\manext $(COMMSOS) > commsos.man
	man2hv $(HTML_EXT) $(VERDOT) COMMSOS commsos.man index$(HTML_EXT)
	.\manext $(QUERY) > query.man
	man2hv $(HTML_EXT) $(VERDOT) QUERY    query.man index$(HTML_EXT)
	.\manext $(GLOSSARY) > glossary.man
	man2hv $(HTML_EXT) $(VERDOT) GLOSSARY glossary.man index$(HTML_EXT)
	.\manext $(APPENDIX1) > app1.man
	man2hv $(HTML_EXT) $(VERDOT) APPENDIX1 app1.man index$(HTML_EXT)
	.\manext $(APPENDIX2) > app2.man
	man2hv $(HTML_EXT) $(VERDOT) APPENDIX2 app2.man index$(HTML_EXT)
	.\manext $(APPENDIX3) > app3.man
	man2hv $(HTML_EXT) $(VERDOT) APPENDIX3 app3.man index$(HTML_EXT)
	.\manext $(APPENDIX4) > app4.man
	man2hv $(HTML_EXT) $(VERDOT) APPENDIX4 app4.man index$(HTML_EXT)
	man2hv $(HTML_EXT) $(VERDOT) APPENDIX5 $(docdir)\app5.htm index$(HTML_EXT)
	man2hv $(HTML_EXT) $(VERDOT) APPENDIX6 $(docdir)\app6.htm index$(HTML_EXT)
	.\manext $(APPENDIX7) > app7.man
	man2hv $(HTML_EXT) $(VERDOT) APPENDIX7 app7.man index$(HTML_EXT)
	copy $(ASRC)\HISTORY history.man
	man2hv $(HTML_EXT) $(VERDOT) HISTORY history.man index$(HTML_EXT)
	copy $(ASRC)\THE_Help.txt quickref.man
	man2hv $(HTML_EXT) $(VERDOT) QUICKREF quickref.man index$(HTML_EXT)
	man2hv $(HTML_EXT) $(VERDOT) TOCEND 'junk' index$(HTML_EXT)
