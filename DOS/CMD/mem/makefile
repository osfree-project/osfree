#
# A Makefile for MEM
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

#32_BITS  = 1

PROJ = mem
DESC = Control file attributes
#defines object file names in format objname.$(O)
OBJS = kitten.$(O) mem.$(O) prf.$(O)
# defines additional options for C compiler

!include $(%ROOT)/mk/all.mk


all: $(PROJ).exe $(PROJ).sym .SYMBOLIC

$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk SYSTEM dos
 @%append $^&.lnk NAME $^&
 @%append $^&.lnk OPTION MAP=$^&.wmp
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
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)dos
