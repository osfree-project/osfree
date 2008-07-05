#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

#32_BITS  = 1

PROJ = fixstrs
DESC = Control file attributes
#defines object file names in format objname.$(O)
OBJS = fixstrs.$(O) tcc2wat.$(O)
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

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

