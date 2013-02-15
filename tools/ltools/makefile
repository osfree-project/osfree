#
# A Makefile for LTools
# (c) osFree project,
# valerius, 2006/10/30
#

OS       = __OS2__
#OS       = UNIX   #Not possible to build with Open Watcom for Linux (build target Linux)
# No need to change anything beyond this line ##########################################
#ADD_COPT is overwritten in tools.mk, so also set it in DEFINES.
DEFINES      = -d$(OS) -dSANITY_CHECKS -d_FILE_OFFSET_BITS=64 -d__WATCOM__ &
               -d_LARGEFILE_SOURCE -d_LARGEFILE64_SOURCE -dNEWOUT -i=. -i=$(WATCOM)$(SEP)h$(SEP)os2

INCLUDES = ldir.h proto.h ext2.h UNIXdisk.h DOSdisk.h makefile
PROJ = ldir
DESC = Generate/convert HDD images

srcfiles = $(p)main$(e)  $(p)readdisk$(e) $(p)dir$(e)  $(p)super$(e) &
           $(p)inode$(e) $(p)group$(e)    $(p)util$(e) $(p)rfs$(e)

!include $(%ROOT)/mk/tools.mk

TARGETS = inst

list = ldir lchange lread lwrite ldel lren lmkdir lln ldrive lcd

inst: $(PATH)ldir.exe
 @for %i in ($(list)) do $(CP) $(MYDIR)cmd$(SEP)%i.cmd $(DEST)
 $(CP) $(PATH)ldir.exe $(DEST)

readdisk.o: UNIXdisk.h DOSdisk.h

