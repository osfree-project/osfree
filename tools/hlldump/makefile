# $Id: makefile,v 1.3 2004/08/16 04:51:55 prokushev Exp $

32_BITS = 1
DIR = $(%ROOT)$(SEP)tools$(SEP)bin
PROJ = hlldump
DESC = Debug information dump facility
OBJS = hll.obj hlldirectory.obj exe.obj hllmodule.obj hllpublics.obj &
       hlllines.obj hllsymbols.obj hlltypes.obj

!include $(%ROOT)/mk/all.mk

TARGETS = $(DIR)$(SEP)$(PROJ).exe

$(DIR)$(SEP)$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
# @%append $^&.lnk system os2v2
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 $(SAY) Linking $^@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
