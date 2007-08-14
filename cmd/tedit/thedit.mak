.SUFFIXES:
CC = cl
SRC = . 
!ifdef DEBUG
CFLAGS = /W4 /Zp /Od /Zi /J /AL /I$(SRC) /I\utility\x\win\h /DSTDC_HEADERS /DHAVE_PROTO  /DMSWIN -c /DUSE_QUERCUS
!else
CFLAGS = /W4 /Zp /O /J /AL /I$(SRC) /I\utility\x\win\h /DSTDC_HEADERS /DHAVE_PROTO  /DMSWIN -c /DUSE_QUERCUS
!endif

OBJ = \
BOX.OBJ        \
COLOUR.OBJ     \
COLUMN.OBJ     \
COMM1.OBJ      \
COMM2.OBJ      \
COMM3.OBJ      \
COMM4.OBJ      \
COMM5.OBJ      \
COMMSET1.OBJ   \
COMMSET2.OBJ   \
COMMSOS.OBJ    \
COMMUTIL.OBJ   \
CURSOR.OBJ     \
DEFAULT.OBJ    \
DIRECTRY.OBJ   \
EDIT.OBJ       \
ERROR.OBJ      \
EXECUTE.OBJ    \
THEMATCH.OBJ   \
GETCH.OBJ      \
MYGETOPT.OBJ   \
LINKED.OBJ     \
MOUSE.OBJ      \
NONANSI.OBJ    \
PREFIX.OBJ     \
QUERY.OBJ      \
PRINT.OBJ      \
RESERVED.OBJ   \
SCROLL.OBJ     \
SHOW.OBJ       \
SORT.OBJ       \
TARGET.OBJ     \
THE.OBJ        \
FILE.OBJ       \
UTIL.OBJ

the.lib: $(OBJ)
 lib @thedit.lnk

COLOUR.obj  : $(SRC)\COLOUR.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
THEMATCH.obj : $(SRC)\THEMATCH.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
COMM1.obj   : $(SRC)\COMM1.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
COMM3.obj   : $(SRC)\COMM3.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
COMM4.obj   : $(SRC)\COMM4.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
COMM5.obj   : $(SRC)\COMM5.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
COMMSET1.obj: $(SRC)\COMMSET1.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
COMMSET2.obj: $(SRC)\COMMSET2.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
COMMSOS.obj : $(SRC)\COMMSOS.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
UTIL.obj    : $(SRC)\UTIL.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
DEFAULT.obj : $(SRC)\DEFAULT.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
DIRECTRY.obj: $(SRC)\DIRECTRY.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
BOX.obj     : $(SRC)\BOX.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
PREFIX.obj  : $(SRC)\PREFIX.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
PRINT.obj  : $(SRC)\PRINT.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
QUERY.obj   : $(SRC)\QUERY.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
EDIT.obj    : $(SRC)\EDIT.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
MYGETOPT.obj  : $(SRC)\MYGETOPT.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
CURSOR.obj  : $(SRC)\CURSOR.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
LINKED.obj  : $(SRC)\LINKED.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
RESERVED.obj: $(SRC)\RESERVED.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
TARGET.obj  : $(SRC)\TARGET.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
COMM2.obj   : $(SRC)\COMM2.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
NONANSI.obj : $(SRC)\NONANSI.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
GETCH.obj   : $(SRC)\GETCH.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
SORT.obj    : $(SRC)\SORT.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
FILE.obj    : $(SRC)\FILE.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
SHOW.obj    : $(SRC)\SHOW.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
EXECUTE.obj : $(SRC)\EXECUTE.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
THE.obj     : $(SRC)\THE.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
COMMUTIL.obj: $(SRC)\COMMUTIL.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
ERROR.obj   : $(SRC)\ERROR.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
SCROLL.obj  : $(SRC)\SCROLL.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
COLUMN.obj  : $(SRC)\COLUMN.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
MOUSE.obj   : $(SRC)\MOUSE.c
  $(CC) $(CFLAGS) $(SRC)\$*.c
