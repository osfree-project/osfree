#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

PROJ = zip
DIR = $(ROOT)\bin\os2
# defines additional options for C compiler
ADD_LINKOPT = lib cmd_shared.lib,all_shared.lib

DESC = ZIP file archiver
OBJS = zip.obj zipfile.obj zipup.obj fileio.obj util.obj &
       crc32.obj crctab.obj globals.obj &
       deflate.obj trees.obj crypt.obj ttyio.obj &
       os2zip.obj os2.obj os2acl.obj

ADD_COPT = -i=..\include -DOS2 -DNO_ASM -DNO_SYMLINK


!include $(%ROOT)/mk/all.mk

all: $(PROJ).exe .SYMBOLIC

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)os2
