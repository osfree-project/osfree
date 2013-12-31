CFLAGS = -fomit-frame-pointer -O1 -DNDEBUG
LDFLAGS = -s -N

VERSION = 1.2
UNI2H = 1

BINDIR = /usr/bin

OBJS = set-boot.o

set-boot: $(OBJS)
	$(CC) -o set-boot $(LDFLAGS) $(OBJS)

install: setboot set-boot
	install -m 700 setboot $(BINDIR)
	install -m 700 set-boot $(BINDIR)

tar:
	(cd ..; tar czf setboot/setboot-$(VERSION).tar.gz \
	 setboot/{README,Makefile,*.[ch],setboot{,.exe}})

clean:
	rm -f $(OBJS) set-boot core

set-boot.o : set-boot.c bootmgr.h 
