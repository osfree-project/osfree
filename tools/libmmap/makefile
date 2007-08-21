#
# A Makefile for libmmap
# (c) osFree project,
# valerius, 2006/10/30
#

!include ../../build.conf
!include ../../mk/site.mk

32_BITS  = 1

# This is the operating system, only LINUX and SOLARIS (x86) are supported
OS       = LINUX
# No need to change anything beyond this line ##########################################
ADD_COPT     = -dUNIX -d$(OS) -dSANITY_CHECKS -d_FILE_OFFSET_BITS=64 &
               -d_LARGEFILE_SOURCE -d_LARGEFILE64_SOURCE -dNEWOUT -i=.

INCLUDES = ldir.h proto.h ext2.h UNIXdisk.h DOSdisk.h makefile

OBJS     = main.$(O)  readdisk.$(O) dir.$(O)  super.$(O) &
           inode.$(O) group.$(O)    util.$(O) rfs.$(O)

!include ../../mk/bootseq.mk

!ifeq UNIX FALSE

all: ldir.exe .SYMBOLIC
 #copy ldir.cmd      ..\bin
 #copy lchange.cmd   ..\bin
 #copy lread.cmd     ..\bin
 #copy lwrite.cmd    ..\bin
 #copy ldel.cmd      ..\bin
 #copy lren.cmd      ..\bin
 #copy lmkdir.cmd    ..\bin
 #copy lln.cmd       ..\bin
 #copy ldrive.cmd    ..\bin
 #copy lcd.cmd       ..\bin

!else

all: ldir.exe .SYMBOLIC
 mv -f ldir      ../bin
 cp -f lchange   ../bin
 cp -f lread     ../bin
 cp -f lwrite    ../bin
 cp -f ldel      ../bin
 cp -f lren      ../bin
 cp -f lmkdir    ../bin
 cp -f lln       ../bin
 cp -f ldrive    ../bin
 cp -f lcd       ../bin
 chmod 755       ../bin/l*

!endif

ldir.exe:   $(OBJS) $(INCLUDES)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk NAME $^&
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)

readdisk.o: UNIXdisk.h
 $(SAY) Compiling $< $(LOG)
 $(CC) $(COPT) -fo=$^&.$(O) readdisk.c $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
