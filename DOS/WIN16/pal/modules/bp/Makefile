LINUX = /usr/src/linux

include $(LINUX)/.config

ifndef CFLAGS
CFLAGS = -O2 -g -Wall -Wstrict-prototypes -Winline -pipe
endif

COFLAGS = -kv
CPPFLAGS = $(DEBUG) -D__KERNEL__ -DMODULE -I$(LINUX)/include -I$(LINUX)

SRCS = bp.c

MODULES = bp.o

all: $(MODULES)

bp.s : bp.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -S bp.c

bp.o : bp.c
	$(CC) -nostdlib -Wl,-r $(CFLAGS) $(CPPFLAGS) -o $@ bp.c
	chmod -x bp.o
