all: vorbis.dll vorbis.lib

include Makefile.am

.SUFFIXES: .obj
SHELL=sh
CC=gcc
CFLAGS_SAFE=-Wall -O6 -Zmtd -Zdll -Zomf -fomit-frame-pointer -finline-functions -I. -I../include -I../../libogg-1.0/include
CFLAGS=$(CFLAGS_SAFE) -funroll-loops
LDFLAGS=-s -L. -L../../libogg-1.0/src -logg

OBJECTS:=$(patsubst %.c,%.obj,$(libvorbis_la_SOURCES) $(libvorbisfile_la_SOURCES) $(libvorbisenc_la_SOURCES))
OBJECTS:=$(patsubst %.h,,$(OBJECTS))

vorbis.lib: $(OBJECTS)
	-rm vorbis.lib
	emxomfar r vorbis.lib $(OBJECTS)

vorbis.dll: $(OBJECTS) vorbis.def
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) vorbis.def $(LDFLAGS)

vorbis.def: $(OBJECTS)
	@echo LIBRARY vorbis INITINSTANCE > $@
	@echo EXPORTS >> $@
	emxexp $(OBJECTS) | grep -E '("vorbis_)|("ov_)' >> $@

.c.obj:
	$(CC) -c $(CFLAGS) $<

# res0_look() gets fouled up by -funroll-loops
res0.obj: res0.c
	$(CC) -c $(CFLAGS_SAFE) $<

clean:
	rm -f *.obj *.dll *.lib *.def

