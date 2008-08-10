# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

32_BITS  = 1

DIR = $(ROOT)$(SEP)tools$(SEP)bin
PROJ = somcpp
DESC = SOM Compiler Preprocessor
OBJS = main.$(O) directive.$(O) eval.$(O) expand.$(O) &
       support.$(O) system.$(O) mbchar.$(O)

ADD_COPT = -i=..$(SEP)include

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ).exe

#Notes:
# awk lex and yacc files not added to repository

$(DIR)$(SEP)$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
# must be native, not OS/2 target
# @%append $^&.lnk SYSTEM os2v2
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=6553500
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 $(SAY) Linking $@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
