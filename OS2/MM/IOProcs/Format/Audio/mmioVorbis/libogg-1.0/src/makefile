all: ogg.dll ogg.lib

include Makefile.am

.SUFFIXES: .obj

CC = gcc
CFLAGS=-O6 -Zmtd -Zdll -Zomf -I../include

OBJ:=$(patsubst %.c,%.obj,$(libogg_la_SOURCES))
OBJ:=$(patsubst %.h,,$(OBJ))

ogg.lib: $(OBJ)
	-rm ogg.lib
	emxomfar r ogg.lib $(OBJ)

ogg.dll: $(OBJ) ogg.def
	$(CC) $(CFLAGS) -s -o $@ $(OBJ) ogg.def

.c.obj:
	$(CC) -c $(CFLAGS) $<

ogg.def: $(OBJ)
	@echo LIBRARY ogg INITINSTANCE > $@
	@echo EXPORTS >> $@
	emxexp $(OBJ) >> $@

clean:
	rm -f *.obj *.dll *.lib *.def
