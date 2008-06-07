CC=gcc
CFLAGS=-O2 -Zomf -DUSE_OS2_TOOLKIT_HEADERS
#CFLAGS=-Wall -g -DDEBUG -Zomf -DUSE_OS2_TOOLKIT_HEADERS
LIBS= -lm -los2me
INCLUDES= -Ilibvorbis-1.0/include -Ilibogg-1.0/include
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CFLAGS)

all: mmioVorb.dll convert.exe bitrate.exe
                                                                 
bitrate.exe: bitrate.c
	$(CC) $(CFLAGS) $^ -o $@ -Zstack 65536 $(INCLUDES) -lm -los2me

%.obj: %.c
	$(COMPILE) -c $<

*.obj: mmioVorbis.h

mmioVorb.dll: mmioVorbis.obj 
	$(CC) $(CFLAGS) -Zdll -Zcrtdll $(LDFLAGS) $(INCLUDES) $^ libvorbis-1.0/lib/vorbis.lib libogg-1.0/src/ogg.lib mmiovorbis.def -o $@ $(LIBS)

convert.exe: convert.c
	$(CC) $(CFLAGS) $^ -o $@ -Zstack 65536 $(INCLUDES) $(LIBS)

mmioVorbis.exe: main.obj
	$(CC) $(CFLAGS) $^ -o $@ -Zstack 65536 $(LIBS)
	
clean:
	-rm core *.obj mmiovorbis.exe mmiovorb.dll convert.exe bitrate.exe

