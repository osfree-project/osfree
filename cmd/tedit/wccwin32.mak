#
#########################################################################
#
# makefile for The Hessling Editor (THE)
#
# You need the following environment variable set like:
# THE_SRCDIR=c:\the
# PDCURSES_BINDIR=c:\pdc
# PDCURSES_SRCDIR=c:\pdcurses
# REGINA_SRCDIR=c:\regina
# REGINA_BINDIR=c:\regina
#
#########################################################################
#

VER = 31
PROJ = proj   # this will get replaced by uncommenting a section
MAN  = man    # this will get replaced by uncommenting a section

#########################################################################
# Watcom  compiler on Win32
#########################################################################
SRC       = $(%THE_SRCDIR)\
WATCOMSRC = $(%WATCOM)\src\startup\
CURSLIB   = $(%PDCURSES_BINDIR)\4r\pdcurses.lib
CURSINC   = /i=$(%PDCURSES_SRCDIR)
REXXINC   = /i=$(%REGINA_SRCDIR) /DUSE_REGINA
REXXLIB   = $(%REGINA_BINDIR)\4r\rexx.lib

PROJ      = the.exe
OBJ       = obj
CC        = wcc386


!ifeq DEBUG Y
CFLAGS    = /zz /d2 /ei /zq /mf /DSTDC_HEADERS /DPROTO /DWIN32 /i=$(SRC) $(CURSINC) $(REXXINC) /fo=$@
LDEBUG     = debug watcom all
!else
CFLAGS    = /zz /oneatx /ei /zq /mf /DSTDC_HEADERS /DPROTO /DWIN32 /i=$(SRC) $(CURSINC) $(REXXINC) /fo=$@
LDEBUG     =
!endif
LD        = wlink
XTRAOBJ   = mygetopt.obj
MAN       = manext.exe
MANLD     = link386 manext+mygetopt+wildargv,,,,ibm-man.def /NOE /NOI /EXEPACK /ALIGN:4;
docdir = doc
#########################################################################
#
#
# Object files
#
OBJ1 = box.$(OBJ) colour.$(OBJ) comm1.$(OBJ) comm2.$(OBJ) comm3.$(OBJ) comm4.$(OBJ) comm5.$(OBJ) &
	commset1.$(OBJ) commset2.$(OBJ) commsos.$(OBJ) cursor.$(OBJ) default.$(OBJ) &
	edit.$(OBJ) error.$(OBJ) execute.$(OBJ) linked.$(OBJ) column.obj mouse.obj memory.obj &
	nonansi.$(OBJ) prefix.$(OBJ) parser.$(OBJ) reserved.$(OBJ) scroll.$(OBJ) show.$(OBJ) sort.$(OBJ) &
	target.$(OBJ) the.$(OBJ) util.$(OBJ) print.$(OBJ) regex.obj alloca.obj
OBJ2 = commutil.$(OBJ) #trace.obj
OBJ3 = getch.$(OBJ)
OBJ4 = query.$(OBJ)
OBJ5 = thematch.$(OBJ)
OBJ6 = directry.$(OBJ) file.$(OBJ)
OBJ7 = rexx.$(OBJ)
OBJ8 = wildargv.obj
OBJX = $(XTRAOBJ)
OBJS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8) $(OBJX)
#########################################################################

COMM = $(SRC)comm1.c $(SRC)comm2.c $(SRC)comm3.c $(SRC)comm4.c $(SRC)comm5.c &
	$(SRC)commsos.c $(SRC)commset1.c $(SRC)commset2.c $(SRC)query.c

APPENDIX = $(SRC)appendix.1
GLOSSARY = $(SRC)glossary

#
#########################################################################
$(PROJ):	$(OBJS)
	@%create dummy.lnk
	@%append dummy.lnk  option quiet
	@%append dummy.lnk  option map
	@%append dummy.lnk  option stack=256k
	@%append dummy.lnk  system nt
	@%append dummy.lnk  name the.exe
	@%append dummy.lnk  file *.obj
	@%append dummy.lnk  library $(CURSLIB)
	@%append dummy.lnk  library $(REXXLIB)
	$(LD) $(LDEBUG) $(LDFLAGS) @dummy.lnk
	del dummy.lnk
#########################################################################
box.obj:	$(SRC)box.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
colour.obj:	$(SRC)colour.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
comm1.obj:	$(SRC)comm1.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
comm2.obj:	$(SRC)comm2.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
comm3.obj:	$(SRC)comm3.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
comm4.obj:	$(SRC)comm4.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
comm5.obj:	$(SRC)comm5.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
commset1.obj:	$(SRC)commset1.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
commset2.obj:	$(SRC)commset2.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
commsos.obj:	$(SRC)commsos.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
cursor.obj:	$(SRC)cursor.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
default.obj:	$(SRC)default.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
edit.obj:	$(SRC)edit.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
error.obj:	$(SRC)error.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
execute.obj:	$(SRC)execute.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
linked.obj:	$(SRC)linked.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
column.obj:	$(SRC)column.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
mouse.obj:	$(SRC)mouse.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
memory.obj:	$(SRC)memory.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
nonansi.obj:	$(SRC)nonansi.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
parser.obj:	$(SRC)parser.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
prefix.obj:	$(SRC)prefix.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
print.obj:	$(SRC)print.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
regex.obj:	$(SRC)\regex.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
alloca.obj:	$(SRC)\alloca.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
reserved.obj:	$(SRC)reserved.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
scroll.obj:	$(SRC)scroll.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
show.obj:	$(SRC)show.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
sort.obj:	$(SRC)sort.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
target.obj:	$(SRC)target.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
the.obj:	$(SRC)the.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
util.obj:	$(SRC)util.c $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
commutil.obj:	$(SRC)commutil.c $(SRC)the.h $(SRC)command.h $(SRC)defines.h $(SRC)proto.h $(SRC)getch.h $(SRC)key.h
	$(CC) $(CFLAGS) $(SRC)$*.c
trace.obj:	$(SRC)trace.c $(SRC)the.h $(SRC)command.h $(SRC)defines.h $(SRC)proto.h $(SRC)getch.h $(SRC)key.h
	$(CC) $(CFLAGS) $(SRC)$*.c
getch.obj:	$(SRC)getch.c $(SRC)getch.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
query.obj:	$(SRC)query.c $(SRC)query.h $(SRC)the.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
thematch.obj:	$(SRC)thematch.c $(SRC)the.h $(SRC)thematch.h
	$(CC) $(CFLAGS) $(SRC)$*.c
directry.obj:	$(SRC)directry.c $(SRC)the.h $(SRC)directry.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
file.obj:	$(SRC)file.c $(SRC)the.h $(SRC)directry.h $(SRC)defines.h $(SRC)proto.h
	$(CC) $(CFLAGS) $(SRC)$*.c
rexx.obj:	$(SRC)rexx.c $(SRC)the.h $(SRC)therexx.h $(SRC)proto.h $(SRC)defines.h $(SRC)query.h
	$(CC) $(CFLAGS) $(SRC)$*.c
mygetopt.obj:	$(SRC)mygetopt.c
	$(CC) $(CFLAGS) $(SRC)$*.c

wildargv.obj:	$(WATCOMSRC)wildargv.c
	$(CC) $(CFLAGS) $(WATCOMSRC)$*.c
#
#########################################################################
manual:	$(MAN) $(SRC)overview $(COMM) $(APPENDIX) $(GLOSSARY)
	manext $(SRC)overview $(COMM) $(APPENDIX) $(GLOSSARY) > the.man
#
$(MAN):	$(XTRAOBJ) manext.$(OBJ)
	$(MANLD)
	$(CHMODMAN)


zip:
	zip thesrc$(VER) README INSTALL TODO COPYING HISTORY THE_Help.txt
	zip thesrc$(VER) overview appendix.1 appendix.2 appendix.3 glossary README.OS2
	zip thesrc$(VER) box.c colour.c comm*.c cursor.c default.c directry.c
	zip thesrc$(VER) edit.c error.c norexx.c scroll.c column.c execute.c
	zip thesrc$(VER) file.c thematch.c getch.c mygetopt.c linked.c mouse.c memory.c
	zip thesrc$(VER) nonansi.c os2eas.c prefix.c query.c reserved.c parser.c
	zip thesrc$(VER) rexx.c show.c sort.c target.c the.c trace.c util.c
	zip thesrc$(VER) command.h defines.h directry.h thematch.h getch.h
	zip thesrc$(VER) key.h query.h proto.h therexx.h the.h makefile.dist $(docdir)/*.gif
	zip thesrc$(VER) manext.c *.rsp *.def *.diz files.rcs the*.xbm icons.zip the.res the.rc the.eas
	zip thesrc$(VER) append.the comm.the uncomm.the total.the match.the rm.the
	zip thesrc$(VER) words.the l.the compile.the spell.the demo.the demo.txt
	zip thesrc$(VER) Makefile.in configure config.h.in $(docdir)/THE_Help*
	zip thesrc$(VER) config.guess config.sub install-sh
	zip thesrc$(VER) aclocal.m4 configure.in
	zip thesrc$(VER) man2html.rex
#
