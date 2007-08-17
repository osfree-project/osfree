#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project.
# valerius, 2006/10/30
#

!include ./build.conf
!include ./mk/site.mk

DIRS = boot # muFSD muIIF filesys freeldr cmd freepm

!include ./mk/bootseq.mk

default: .SYMBOLIC
 $(MAKE) $(MAKEOPT) all

all: .SYMBOLIC
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

tools: .SYMBOLIC
 cd tools && $(MAKE) $(MAKEOPT) all && cd ..

install: fdd hdd .SYMBOLIC

!ifeq UNIX TRUE

hdd: .SYMBOLIC
 $(DC) $(IMGDIR)/hdd1.img $(BLACKHOLE)
 $(DC) start.img $(BLACKHOLE)
 $(DC) $(IMGDIR)/hdd.img $(BLACKHOLE)
 $(DC) $(FILESDIR)/os2ldr $(BLACKHOLE)
 $(DC) $(FILESDIR)/boot/bb_ext2 $(BLACKHOLE)
 $(CP) $(ROOT)/freeldr/os2ldr $(FILESDIR) $(BLACKHOLE)
 $(CP) $(ROOT)/muFSD/ext2/bb_ext2 $(FILESDIR)/boot $(BLACKHOLE)
 $(GENE2FS) -d $(FILESDIR) -b 800 hdd1.img
 $(GENHDD)  -p hdd1.img                                &
            -o $(IMGDIR)/hdd.img                       &
            -m $(ROOT)/boot/mbr/mbr.bin                &
            -b $(ROOT)/boot/bootsect/bootsect_1.bin    &
            -B $(TOOLS)/bpb.cfg                        &
            -f \"$(ROOT)/muFSD/ext2/bb_ext2 $(ROOT)/hdd1.img\"
 $(DC) hdd1.img $(BLACKHOLE)
 $(DC) start.img $(BLACKHOLE)
 $(DC) zeroes.bin $(BLACKHOLE)

fdd: .SYMBOLIC
 $(DC) $(IMGDIR)/flp.img $(BLACKHOLE)
 $(DC) $(FILESDIR)/os2ldr $(BLACKHOLE)
 $(DC) $(FILESDIR)/boot/bb_ext2 $(BLACKHOLE)
 $(CP) $(ROOT)/freeldr/os2ldr $(FILESDIR) $(BLACKHOLE)
 $(CP) $(ROOT)/muFSD/ext2/bb_ext2 $(FILESDIR)/boot $(BLACKHOLE)
 $(GENE2FS) -d $(FILESDIR) -b 1440 flp.img
 $(GENFDD)  -i flp.img                                 &
            -b $(ROOT)/boot/bootsect/bootsect_1.bin    &
            -B $(TOOLS)/diskette.cfg                   &
            -f \"$(ROOT)/muFSD/ext2/bb_ext2 $(ROOT)/flp.img\"
 $(RN) flp.img $(IMGDIR)

!else

hdd: .SYMBOLIC
 $(DC) $(IMGDIR)\hdd1.img $(BLACKHOLE)
 $(DC) start.img $(BLACKHOLE)
 $(DC) $(IMGDIR)\hdd.img $(BLACKHOLE)
 $(DC) $(FILESDIR)\os2ldr $(BLACKHOLE)
 $(DC) $(FILESDIR)\boot\bb_ext2 $(BLACKHOLE)
 $(CP) $(ROOT)\freeldr\os2ldr $(FILESDIR) $(BLACKHOLE)
 $(CP) $(ROOT)\muFSD\ext2\bb_ext2 $(FILESDIR)\boot $(BLACKHOLE)
 $(GENE2FS) -d $(FILESDIR1) -b 800 hdd1.img
 $(GENHDD)  -p hdd1.img                               &
            -o $(IMGDIR)\hdd.img                      &
            -m $(ROOT)\boot\mbr\mbr.bin               &
            -b $(ROOT)\boot\bootsect\bootsect_1.bin   &
            -B $(TOOLS)\bpb.cfg                       &
            -f "$(ROOT)\muFSD\ext2\bb_ext2 $(ROOT)\hdd1.img"
 $(DC) hdd1.img $(BLACKHOLE)
 $(DC) start.img $(BLACKHOLE)
 $(DC) zeroes.bin $(BLACKHOLE)

fdd: .SYMBOLIC
 $(DC) $(IMGDIR)\flp.img $(BLACKHOLE)
 $(DC) $(FILESDIR)\os2ldr $(BLACKHOLE)
 $(DC) $(FILESDIR)\boot\bb_ext2 $(BLACKHOLE)
 $(CP) $(ROOT)\freeldr\os2ldr $(FILESDIR) $(BLACKHOLE)
 $(CP) $(ROOT)\muFSD\ext2\bb_ext2 $(FILESDIR)\boot $(BLACKHOLE)
 $(GENE2FS) -d $(FILESDIR1) -b 1440 flp.img
 $(GENFDD)  -i flp.img                                 &
            -b $(ROOT)\boot\bootsect\bootsect_1.bin    &
            -B $(TOOLS)\diskette.cfg                   &
            -f "$(ROOT)\muFSD\ext2\bb_ext2 $(ROOT)\flp.img"
 $(RN) flp.img $(IMGDIR)

!endif

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
