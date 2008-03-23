#Directory where mzloader is found
MZLOADER_DIR=/usr/sbin

#Directory where module is found
MODDIR=/lib/modules/$(shell uname -r)/misc

CC=	gcc
MODFLAGS=	-O2 -Wall -D__KERNEL__ -DMODULE -DMZLOADER=\"${MZLOADER_DIR}/mzloader\"

all:	binfmt_mz.o mzloader

binfmt_mz.o:	binfmt_mz.c
	$(CC) $(MODFLAGS) -c binfmt_mz.c

mzloader:	mzloader.c
	$(CC) $(ALLCFLAGS) -o mzloader mzloader.c

install:	all
	install binfmt_mz.o ${MODDIR}
	install mzloader ${MZLOADER_DIR}

clean::
	rm -f *.o mzloader core
