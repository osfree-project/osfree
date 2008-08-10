# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

32_BITS  = 1

DIR = $(ROOT)$(SEP)tools$(SEP)bin
PROJ = yacc
DESC = Parser generator
OBJS = LR0.$(O) allocate.$(O) closure.$(O) conflicts.$(O) &
derives.$(O) files.$(O) getargs.$(O) gram.$(O) lalr.$(O) lex.$(O) main.$(O) &
nullable.$(O) output.$(O) print.$(O) reader.$(O) reduce.$(O) symtab.$(O) &
warshall.$(O) getopt.$(O) getopt1.$(O)

ADD_COPT = -i=..$(SEP)include -dHAVE_CONFIG_H

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
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 $(SAY) Linking $@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
