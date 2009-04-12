VERSION=2.0

DESTDIR=/usr/local
BINDIR=$(DESTDIR)/bin/
MANDIR=$(DESTDIR)/man/man1/
DATADIR=./

CC = gcc
CFLAGS = -O2 -Wall -DOS2
CPPFLAGS = -DVERSION=\"$(VERSION)\" -DDATADIR=\"$(DATADIR)\"

DB_OBJS = fileio.o util.o dbench.o child.o
TB_OBJS = sockio.o util.o dbench.o child.o socklib.o
SRV_OBJS = util.o tbench_srv.o socklib.o

all: dbench.exe
really_all: dbench tbench tbench_srv

dbench.exe: $(DB_OBJS)
	$(CC) -o $@ $(DB_OBJS)

tbench: $(TB_OBJS)
	$(CC) -o $@ $(TB_OBJS)

tbench_srv: $(SRV_OBJS)
	$(CC) -o $@ $(SRV_OBJS)

# Careful here: don't install client.txt over itself.
#install: all
#	install -d $(BINDIR) $(DATADIR) $(MANDIR)
#	install dbench tbench tbench_srv $(BINDIR)
#	install -m644 dbench.1 $(MANDIR)
#	ln -s dbench.1 $(MANDIR)/tbench.1
#	ln -s dbench.1 $(MANDIR)/tbench_srv.1
#	[ "$(DATADIR)" = ./ ] || install -m644 client.txt $(DATADIR)

clean:
	rm -f *.o *~ dbench tbench tbench_srv

proto:
	cat *.c | awk -f mkproto.awk > proto.h
