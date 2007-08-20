# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

!include ../../build.conf
!include ../../mk/site.mk

ROOT= ..$(SEP)..
32_BITS  = 1

DIR = $(ROOT)$(SEP)tools$(SEP)bin
PROJ = awk
DESC = AWK language interpreter
OBJS = b.$(O) main.$(O) parse.$(O) proctab.$(O) tran.$(O) lib.$(O) run.$(O) lex.$(O) &
       ytab.$(O) missing95.$(O)

ADD_COPT = -i=..$(SEP)include
ADD_LIB = clib3r,math387r

!include ../../mk/cmd.mk

all: $(DIR)$(SEP)$(PROJ).exe $(DIR)$(SEP)$(PROJ).sym

$(DIR)$(SEP)$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk FORMAT OS2 LX PMCOMPATIBLE
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 @%append $^&.lnk LIBPATH $(LIBPATH)
 @%append $^&.lnk LIBRARY $(LINKLIB)
 $(SAY) Linking $@
 @$(LINKER) @$^&.lnk $(LOG)

# Clean all

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
