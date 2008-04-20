#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project.
# valerius, 2006/10/30
#

!include ./build.conf
!include ./mk/site.mk

DIR  = $(%ROOT)$(SEP)bin
DIRS = tools bootseq OS2 DOS

!include ./mk/all.mk

.DEFAULT
all: .SYMBOLIC
 @$(if_not_exist_mkdir) $(DIR)
 @$(if_not_exist_mkdir) $(DIR)$(SEP)os2
 @$(if_not_exist_mkdir) $(DIR)$(SEP)os2$(SEP)nls mkdir
 @$(if_not_exist_mkdir) $(DIR)$(SEP)os2$(SEP)nls$(SEP)book
 @$(if_not_exist_mkdir) $(DIR)$(SEP)os2$(SEP)nls$(SEP)msg
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

install: fdd hdd .SYMBOLIC
 @$(MAKE) $(MAKEOPT) TARGET=$^@ subdirs

!ifeq UNIX TRUE

hdd: .SYMBOLIC
 $(DC) $(IMGDIR)/hdd1.img $(BLACKHOLE)
 $(DC) start.img $(BLACKHOLE)
 $(DC) $(IMGDIR)/hdd.img $(BLACKHOLE)
 $(DC) $(FILESDIR)/os2ldr $(BLACKHOLE)
 $(DC) $(FILESDIR)/boot/bb_ext2 $(BLACKHOLE)
 $(CP) $(ROOT)/freeldr/os2ldr $(FILESDIR) $(BLACKHOLE)
 $(CP) $(ROOT)/bootseq/uFSD/ext2/bb_ext2 $(FILESDIR)/boot $(BLACKHOLE)
 $(GENE2FS) -d $(FILESDIR) -b 800 hdd1.img
 $(GENHDD)  -p hdd1.img                                &
            -o $(IMGDIR)/hdd.img                       &
            -m $(ROOT)/bootseq/bootsec/mbr/mbr.bin                &
            -b $(ROOT)/bootseq/bootsec/bootsect/bootsect_1.bin    &
            -B $(TOOLS)/bpb.cfg                        &
            -f \"$(ROOT)/bootseq/uFSD/ext2/bb_ext2 $(ROOT)/hdd1.img\"
 $(DC) hdd1.img $(BLACKHOLE)
 $(DC) start.img $(BLACKHOLE)
 $(DC) zeroes.bin $(BLACKHOLE)

fdd: .SYMBOLIC
 $(DC) $(IMGDIR)/flp.img $(BLACKHOLE)
 $(DC) $(FILESDIR)/os2ldr $(BLACKHOLE)
 $(DC) $(FILESDIR)/boot/bb_ext2 $(BLACKHOLE)
 $(CP) $(ROOT)/bootseq/freeldr/os2ldr $(FILESDIR)/OS2LDR $(BLACKHOLE)
 $(CP) $(ROOT)/bootseq/uFSD/ext2/bb_ext2 $(FILESDIR)/boot $(BLACKHOLE)
 $(GENE2FS) -d $(FILESDIR) -b 1440 flp.img
 $(GENFDD)  -i flp.img                                 &
            -b $(ROOT)/bootseq/bootsec/bootsect/bootsect_1.bin    &
            -B $(TOOLS)/diskette.cfg                   &
            -f \"$(ROOT)/bootseq/uFSD/ext2/bb_ext2 $(ROOT)/flp.img\"
 $(RN) flp.img $(IMGDIR)

!else

hdd: .SYMBOLIC
 $(DC) $(IMGDIR)\hdd1.img $(BLACKHOLE)
 $(DC) start.img $(BLACKHOLE)
 $(DC) $(IMGDIR)\hdd.img $(BLACKHOLE)
 $(DC) $(FILESDIR)\os2ldr $(BLACKHOLE)
 $(DC) $(FILESDIR)\boot\bb_ext2 $(BLACKHOLE)
 $(CP) $(ROOT)\bootseq\freeldr\os2ldr $(FILESDIR) $(BLACKHOLE)
 $(CP) $(ROOT)\bootseq\uFSD\ext2\bb_ext2 $(FILESDIR)\boot $(BLACKHOLE)
 $(GENE2FS) -d $(FILESDIR1) -b 800 hdd1.img
 $(GENHDD)  -p hdd1.img                               &
            -o $(IMGDIR)\hdd.img                      &
            -m $(ROOT)\bootseq\bootsec\mbr\mbr.bin               &
            -b $(ROOT)\bootseq\bootsec\bootsect\bootsect_1.bin   &
            -B $(TOOLS)\bpb.cfg                       &
            -f "$(ROOT)\bootseq\uFSD\ext2\bb_ext2 $(ROOT)\hdd1.img"
 $(DC) hdd1.img $(BLACKHOLE)
 $(DC) start.img $(BLACKHOLE)
 $(DC) zeroes.bin $(BLACKHOLE)

fdd: .SYMBOLIC
 $(DC) $(IMGDIR)\flp.img $(BLACKHOLE)
 $(DC) $(FILESDIR)\os2ldr $(BLACKHOLE)
 $(DC) $(FILESDIR)\boot\bb_ext2 $(BLACKHOLE)
 $(CP) $(ROOT)\bootseq\freeldr\os2ldr $(FILESDIR) $(BLACKHOLE)
 $(CP) $(ROOT)\bootseq\uFSD\ext2\bb_ext2 $(FILESDIR)\boot $(BLACKHOLE)
 $(GENE2FS) -d $(FILESDIR1) -b 1440 flp.img
 $(GENFDD)  -i flp.img                                 &
            -b $(ROOT)\bootseq\bootsec\bootsect\bootsect_1.bin    &
            -B $(TOOLS)\diskette.cfg                   &
            -f "$(ROOT)\bootseq\uFSD\ext2\bb_ext2 $(ROOT)\flp.img"
 $(RN) flp.img $(IMGDIR)

!endif

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 cd lib
 $(CLEAN_CMD)
 cd ..
 $(MAKE) $(MAKEOPT) TARGET=$^@ subdirs
