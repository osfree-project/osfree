.SUFFIXES: .obj
CC      = gcc
CFLAGS  = -Zmtd -Zomf -Wall -Iinclude -I../../vorbis/include -I../../ogg/include
LD      = gcc
LDFLAGS = -s -Zmtd -Zomf -Zdll -L../../vorbis/lib -L../../ogg/src


all: OggPlay.dll

.c.obj:
	$(CC) -c $(CFLAGS) $<

OggPlay.dll : oggplay.obj command.obj OggPlay.def
	$(LD) $(LDFLAGS) -o $@ OggPlay.obj command.obj OggPlay.def -lvorbis -logg

clean:
	rm -f *.obj *.dll

oggplay.obj: include/format.h include/decoder_plug.h include/plugin.h include/include.h
command.obj: include/format.h include/decoder_plug.h include/plugin.h include/include.h
