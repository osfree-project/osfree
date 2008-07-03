#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

PROJ = keyb
DESC = Change keyboard layout
#defines object file names in format objname.$(O)
OBJS = keyb.$(O)
# defines additional options for C compiler
ADD_COPT = -i=..$(SEP)include
ADD_LINKOPT = lib cmd_shared.lib,all_shared.lib

!include $(%ROOT)/mk/all.mk


all: $(PROJ).exe $(PROJ).sym .SYMBOLIC

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk OPTION STUB=$(FILESDIR)$(SEP)dos$(SEP)stub.exe
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 @$(MAKE) $(MAKEOPT) install

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)os2
