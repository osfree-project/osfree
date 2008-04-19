## Makefile for building the GLib dlls with Microsoft C
## Use: nmake -f makefile.msc

PARTS = glib gmodule gthread gobject gio tests

all : &
        config.h &
        glibconfig.h &
        sub-all

sub-all:
        for %d in ($(PARTS)) do wmake sub-one THIS=%d TARGET=all

clean : sub-clean

sub-clean:
        for %d in ($(PARTS)) do wmake sub-one THIS=%d TARGET=clean

sub-one:
        @cd $(THIS)
        @wmake $(TARGET)
        @cd ..

