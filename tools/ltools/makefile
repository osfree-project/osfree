#
# A Makefile for LTools
# (c) osFree project,
# valerius, 2006/10/30
#

!ifeq %OS OS2
defs = -d__OS2__ -i=$(WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
defs = -d__WIN32__ -i=$(WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS DOS
defs = -d__DOS__ -i=$(WATCOM)$(SEP)h$(SEP)dos
!endif
!ifeq %OS LINUX
defs = -dUNIX -i=$(WATCOM)$(SEP)lh
!endif

#OS       = __OS2__
#OS       = UNIX   #Not possible to build with Open Watcom for Linux (build target Linux)
# No need to change anything beyond this line ##########################################
#ADD_COPT is overwritten in tools.mk, so also set it in DEFINES.

DEFINES      = $(defs) -dSANITY_CHECKS -d_FILE_OFFSET_BITS=64 -d__WATCOM__ &
               -d_LARGEFILE_SOURCE -d_LARGEFILE64_SOURCE -dNEWOUT -i=.

INCLUDES = ldir.h proto.h ext2.h UNIXdisk.h DOSdisk.h makefile

PROJ = ldir
DESCRIPTION = Generate/convert HDD images

SOURCES  = main readdisk dir super inode group util rfs
ADD_COPT = $(DEFINES)

TARGET_API = HOST
TARGET_BITS = 32

!include $(%ROOT)tools/mk/build.mk

TARGETS = inst

list = ldir lchange lread lwrite ldel lren lmkdir lln ldrive lcd

inst: $(PATH)ldir.exe
 @for %i in ($(list)) do $(CP) $(MYDIR)cmd$(SEP)%i.cmd $(DEST)
 $(CP) $(PATH)ldir.exe $(DEST)

readdisk.o: UNIXdisk.h DOSdisk.h
