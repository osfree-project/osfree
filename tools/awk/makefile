# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

DIR = $(ROOT)$(SEP)tools$(SEP)bin
PROJ = awk
DESC = AWK language interpreter
OBJS = b.$(O) main.$(O) parse.$(O) proctab.$(O) tran.$(O) lib.$(O) run.$(O) lex.$(O) &
       ytab.$(O) missing95.$(O)

ADD_COPT = -i=..$(SEP)include
#LINKOPT = LIB clib3r,math387r

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ).exe $(DIR)$(SEP)$(PROJ).sym

$(DIR)$(SEP)$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 $(SAY) Linking $@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

# Clean all

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
