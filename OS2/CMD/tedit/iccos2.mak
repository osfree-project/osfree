#
#########################################################################
#
# makefile for The Hessling Editor (THE)
#
#########################################################################
#

VER = 31
VERDOT = 3.1
SRC       = c:\the
CURSBIN   = c:\pdc\icc
CURSLIB   = $(CURSBIN)\pdcurses.lib
CURSINC   = -Ic:\curses
SETARGV   = c:\ibmc\lib\setargv.obj
REGINA_BIN = c:\regina
REGINA_REXXLIBS = $(REGINA_BIN)\regina.lib
REGINA_REXXINC = -Ic:\regina -DUSE_REGINA
OS2REXX_REXXLIBS = rexx.lib
OS2REXX_REXXINC = -DUSE_OS2REXX

#########################################################################
# C Set/2 compiler on OS/2
#########################################################################

!if "$(INT)" == "REGINA"
REXXLIB = $(REGINA_REXXLIBS)
REXXINC =  $(REGINA_REXXINC)
!else
!if "$(INT)" == "OS2REXX"
REXXLIB = $(OS2REXX_REXXLIBS)
REXXINC =  $(OS2REXX_REXXINC)
!else
!error Rexx Interpreter NOT specified via INT macro. Valid values are: REGINA OS2REXX
!endif
!endif

!ifdef DEBUG
CFLAGS    = -c -O- -Ti -Sm -Q+ -DSTDC_HEADERS -DHAVE_PROTO -I$(SRC) $(CURSINC) $(REXXINC)
LDEBUG    = /CO
TRACE     = trace.obj
!else
CFLAGS    = -c -O -J+ -Sm -Q+ -DSTDC_HEADERS -DHAVE_PROTO -I$(SRC) $(CURSINC) $(REXXINC)
LDEBUG    = /EXEPACK:2
TRACE     =
!endif

LDFLAGS   = /BAT /SE:160 /NOE /BASE:0x10000 /MAP /NOI /ALIGN:4 $(LDEBUG)

CC        = icc
THERES    = $(SRC)\icc-the.res
THEDEF    = $(SRC)\icc-the.def
LD        = link386 $(LDFLAGS) @the.rsp
MAN       = manext.exe
MANLD     = link386 manext+mygetopt+g:\ibmc\lib\setargv,,,,ibm-man.def /NOE /NOI /EXEPACK:2 /PACKCODE /PACKDATA /ALIGN:4;
RCTHE     = rc $(THERES) the.exe
LXLITE    = lxlite the.exe
#########################################################################
#
#
# Object files
#
OBJ1A = box.obj colour.obj comm1.obj comm2.obj comm3.obj comm4.obj comm5.obj
OBJ1B = commset1.obj commset2.obj commsos.obj cursor.obj default.obj
OBJ1C = edit.obj error.obj execute.obj linked.obj column.obj mouse.obj
OBJ1D = nonansi.obj prefix.obj reserved.obj scroll.obj show.obj sort.obj
OBJ1E = memory.obj target.obj the.obj util.obj
OBJ1 = $(OBJ1A) $(OBJ1B) $(OBJ1C) $(OBJ1D) $(OBJ1E)
OBJ2 = commutil.obj print.obj $(TRACE)
OBJ3 = getch.obj
OBJ4 = query.obj
OBJ5 = thematch.obj
OBJ6 = directry.obj file.obj
OBJ7 = rexx.obj
OBJ8 = mygetopt.obj os2eas.obj
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
	@echo $(OBJ1A) + > the.rsp
	@echo $(OBJ1B) + >> the.rsp
	@echo $(OBJ1C) + >> the.rsp
	@echo $(OBJ1D) + >> the.rsp
	@echo $(OBJ1E) + >> the.rsp
	@echo $(OBJOTH) + >> the.rsp
	@echo $(SETARGV)  >> the.rsp
	@echo the.exe >> the.rsp
	@echo the.map >> the.rsp
	@echo $(REXXLIB)+$(CURSLIB) >> the.rsp
	@echo $(THEDEF) >> the.rsp
	$(LD)
	$(RCTHE)
	$(LXLITE)
	del the.rsp
#########################################################################
box.obj:	$(SRC)\box.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
colour.obj:	$(SRC)\colour.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
comm1.obj:	$(SRC)\comm1.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
comm2.obj:	$(SRC)\comm2.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
comm3.obj:	$(SRC)\comm3.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
comm4.obj:	$(SRC)\comm4.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
comm5.obj:	$(SRC)\comm5.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
commset1.obj:	$(SRC)\commset1.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
commset2.obj:	$(SRC)\commset2.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
commsos.obj:	$(SRC)\commsos.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
cursor.obj:	$(SRC)\cursor.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
default.obj:	$(SRC)\default.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
edit.obj:	$(SRC)\edit.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
error.obj:	$(SRC)\error.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
execute.obj:	$(SRC)\execute.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
linked.obj:	$(SRC)\linked.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
column.obj:	$(SRC)\column.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
mouse.obj:	$(SRC)\mouse.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
memory.obj:	$(SRC)\memory.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
nonansi.obj:	$(SRC)\nonansi.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
prefix.obj:	$(SRC)\prefix.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
print.obj:	$(SRC)\print.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
reserved.obj:	$(SRC)\reserved.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
scroll.obj:	$(SRC)\scroll.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
show.obj:	$(SRC)\show.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
sort.obj:	$(SRC)\sort.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
target.obj:	$(SRC)\target.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
the.obj:	$(SRC)\the.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
util.obj:	$(SRC)\util.c $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
commutil.obj:	$(SRC)\commutil.c $(SRC)\the.h $(SRC)\command.h $(SRC)\defines.h $(SRC)\proto.h $(SRC)\getch.h $(SRC)\key.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
trace.obj:	$(SRC)\trace.c $(SRC)\the.h $(SRC)\command.h $(SRC)\defines.h $(SRC)\proto.h $(SRC)\getch.h $(SRC)\key.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
getch.obj:	$(SRC)\getch.c $(SRC)\getch.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
query.obj:	$(SRC)\query.c $(SRC)\query.h $(SRC)\the.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
thematch.obj:	$(SRC)\thematch.c $(SRC)\the.h $(SRC)\thematch.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
directry.obj:	$(SRC)\directry.c $(SRC)\the.h $(SRC)\directry.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
file.obj:	$(SRC)\file.c $(SRC)\the.h $(SRC)\directry.h $(SRC)\defines.h $(SRC)\proto.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
rexx.obj:	$(SRC)\rexx.c $(SRC)\the.h $(SRC)\therexx.h $(SRC)\proto.h $(SRC)\defines.h $(SRC)\query.h
	$(CC) $(CFLAGS) $(SRC)\$*.c
mygetopt.obj:	$(SRC)\mygetopt.c
	$(CC) $(CFLAGS) $(SRC)\$*.c
os2eas.obj:	$(SRC)\os2eas.c
	$(CC) $(CFLAGS) $(SRC)\$*.c
########################################################################
manual:	$(MAN) $(SRC)/overview $(COMM) $(APPENDIX) $(GLOSSARY)
	manext $(SRC)/overview $(COMM) $(APPENDIX) $(GLOSSARY) > the.man
#
$(MAN):	$(XTRAOBJ) manext.obj
	$(MANLD)
	$(CHMODMAN)

#########################################################################
dist: the.exe
	-mkdir tmp
	cd tmp
	-del /Y *.*
	copy ..\the.exe
	copy $(SRC)\THE_Help.txt .
	copy $(SRC)\demo.txt .
	copy $(SRC)\*.the .
	copy $(SRC)\COPYING .
	copy $(SRC)\HISTORY .
	copy $(SRC)\TODO .
	copy $(SRC)\README .
	copy $(SRC)\README.OS2 .
	copy $(SRC)\the.eas .
	copy $(SRC)\os2.diz file_id.diz
	the -b -p $(SRC)\fix.diz -a "$(VER) $(VERDOT) O built-in Rexx interpreter" file_id.diz
	zip the$(VER)O_os2 *
	cd ..
#########################################################################
