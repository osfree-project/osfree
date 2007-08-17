#
#########################################################################
#
# makefile for The Hessling Editor (THE)
#
#########################################################################
#

VER = 31
SRC       = c:/the
ASRC      = c:\the
CURSBIN   = c:/pdc/djg
CURSLIB   = $(CURSBIN)/pdcurses.a
CURSINC   = -Ic:/curses
REGINA_BIN = c:/regina/djg
REGINA_REXXLIBS = $(REGINA_BIN)/libregina.a
REGINA_REXXINC = -Ic:/regina -DUSE_REGINA
DJGPP_HOME = c:/djgpp
ADJGPP_HOME = c:\djgpp
WATTCPLIB = -Lc:/wattcp/lib -lwatt

#########################################################################
# DJGPP compiler on DOS
#########################################################################

REXXLIB = $(REGINA_REXXLIBS)
REXXINC =  $(REGINA_REXXINC)

ifeq ($(DEBUG),Y)
CFLAGS    = -c -g -Wall -DSTDC_HEADERS -DUSE_REGINA -DHAVE_PROTO -I$(SRC) $(CURSINC) $(REXXINC) -o$*.o
LDEBUG    = -g
TRACE     = trace.o
else
CFLAGS    = -c -O -Wall -DSTDC_HEADERS -DUSE_REGINA -DHAVE_PROTO -I$(SRC) $(CURSINC) $(REXXINC) -o$*.o
LDEBUG    = 
TRACE     =
endif

LDFLAGS   = $(LDEBUG)

CC        = gcc
THERES    = $(SRC)\icc-the.res
THEDEF    = $(SRC)\icc-the.def
LD        = $(CC) $(LDEBUG) -o the @$(SRC)/dos-go32.rsp -L$(DJGPP_HOME)/lib $(CURSLIB) $(REXXLIB) $(WATTCPLIB)
MAN       = manext.exe
MANLD     = link386 manext+mygetopt+g:\ibmc\lib\setargv,,,,ibm-man.def /NOE /NOI /EXEPACK:2 /PACKCODE /PACKDATA /ALIGN:4;
#########################################################################
#
#
# Object files
#
OBJ1A = box.o colour.o comm1.o comm2.o comm3.o comm4.o comm5.o
OBJ1B = commset1.o commset2.o commsos.o cursor.o default.o
OBJ1C = edit.o error.o execute.o linked.o column.o mouse.o
OBJ1D = nonansi.o prefix.o reserved.o scroll.o show.o sort.o
OBJ1E = memory.o target.o the.o util.o parser.o
OBJ1 = $(OBJ1A) $(OBJ1B) $(OBJ1C) $(OBJ1D) $(OBJ1E)
OBJ2 = commutil.o print.o $(TRACE)
OBJ3 = getch.o
OBJ4 = query.o
OBJ5 = thematch.o regex.o mygetopt.o
OBJ6 = directry.o file.o
OBJ7 = rexx.o
OBJ8 = 
OBJOTH = $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8)
OBJS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8)

COMM = $(SRC)/comm1.c $(SRC)/comm2.c $(SRC)/comm3.c $(SRC)/comm4.c $(SRC)/comm5.c \
	$(SRC)/commsos.c $(SRC)/commset1.c $(SRC)/commset2.c $(SRC)/query.c

APPENDIX = $(SRC)/appendix.1 $(SRC)/appendix.2 $(SRC)/appendix.3
GLOSSARY = $(SRC)/glossary

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
	strip the
	coff2exe the
	del the.rsp
#########################################################################
box.o:	$(SRC)/box.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
colour.o:	$(SRC)/colour.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
comm1.o:	$(SRC)/comm1.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
comm2.o:	$(SRC)/comm2.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
comm3.o:	$(SRC)/comm3.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
comm4.o:	$(SRC)/comm4.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
comm5.o:	$(SRC)/comm5.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
commset1.o:	$(SRC)/commset1.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
commset2.o:	$(SRC)/commset2.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
commsos.o:	$(SRC)/commsos.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
cursor.o:	$(SRC)/cursor.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
default.o:	$(SRC)/default.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
edit.o:	$(SRC)/edit.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
error.o:	$(SRC)/error.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
execute.o:	$(SRC)/execute.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
linked.o:	$(SRC)/linked.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
column.o:	$(SRC)/column.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
mouse.o:	$(SRC)/mouse.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
memory.o:	$(SRC)/memory.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
nonansi.o:	$(SRC)/nonansi.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
prefix.o:	$(SRC)/prefix.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
print.o:	$(SRC)/print.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
reserved.o:	$(SRC)/reserved.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
scroll.o:	$(SRC)/scroll.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
show.o:	$(SRC)/show.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
sort.o:	$(SRC)/sort.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
target.o:	$(SRC)/target.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
the.o:	$(SRC)/the.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
util.o:	$(SRC)/util.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
commutil.o:	$(SRC)/commutil.c $(SRC)/the.h $(SRC)/command.h $(SRC)/defines.h $(SRC)/proto.h $(SRC)/getch.h $(SRC)/key.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
trace.o:	$(SRC)/trace.c $(SRC)/the.h $(SRC)/command.h $(SRC)/defines.h $(SRC)/proto.h $(SRC)/getch.h $(SRC)/key.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
getch.o:	$(SRC)/getch.c $(SRC)/getch.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
query.o:	$(SRC)/query.c $(SRC)/query.h $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
thematch.o:	$(SRC)/thematch.c $(SRC)/the.h $(SRC)/thematch.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
parser.o:	$(SRC)/parser.c $(SRC)/the.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
directry.o:	$(SRC)/directry.c $(SRC)/the.h $(SRC)/directry.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
file.o:	$(SRC)/file.c $(SRC)/the.h $(SRC)/directry.h $(SRC)/defines.h $(SRC)/proto.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
rexx.o:	$(SRC)/rexx.c $(SRC)/the.h $(SRC)/therexx.h $(SRC)/proto.h $(SRC)/defines.h $(SRC)/query.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
regex.o:	$(SRC)/regex.c $(SRC)/regex.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
mygetopt.o:	$(SRC)/mygetopt.c $(SRC)/mygetopt.h
	$(CC) $(CFLAGS) $(SRC)/$*.c
########################################################################
manual:	$(MAN) $(SRC)/overview $(COMM) $(APPENDIX) $(GLOSSARY)
	manext $(SRC)/overview $(COMM) $(APPENDIX) $(GLOSSARY) > the.man
#
$(MAN):	$(XTRAOBJ) manext.o
	$(MANLD)
	$(CHMODMAN)

#########################################################################
dist: the.exe
	echo run $(ASRC)\makedist.cmd
#########################################################################
