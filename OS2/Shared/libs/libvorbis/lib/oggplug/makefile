CC      = icc
CFLAGS  = /Iinclude /Ivorbis /Ti /Gm /Gd- /Ge- /qtune=pentium2 /Oi9999 /Oi+ /O+
LD      = ilink
LDFLAGS = /de /dll


all: OggPlay.dll

.c.obj:
        $(CC) $(CFLAGS) /C+ /Fo"%|dpfF.obj" %s

OggPlay.dll : OggPlay.obj command.obj OggPlay.def vorbis/Vorbis.lib
    $(LD) $(LDFLAGS) -out:$@ $**

OggPlay.obj : OggPlay.c

command.obj : command.c

clean :
    -del *.obj OggPlay.dll 2>nul:
