# $Id: makefile,v 1.3 2004/08/16 04:51:55 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

DIR = $(ROOT)$(SEP)tools
PROJ = hlldump
DESC = Debug information dump facility
OBJS = hll.obj hlldirectory.obj exe.obj hllmodule.obj hllpublics.obj &
       hlllines.obj hllsymbols.obj hlltypes.obj

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ).exe $(DIR)$(SEP)$(PROJ).sym

$(DIR)$(SEP)$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk FORMAT OS2 LX PMCOMPATIBLE
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk LIBRARY $(LINKLIB)
 $(ADDFILES_CMD)
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)

# Clean all

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
