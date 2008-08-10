# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

32_BITS  = 1

DIR = $(ROOT)$(SEP)tools$(SEP)bin
PROJ = lex
DESC = Lexical analyzer
OBJS = ccl.$(O) dfa.$(O) ecs.$(O) gen.$(O) main.$(O) misc.$(O) &
       nfa.$(O) parse.$(O) scan.$(O) skel.$(O) sym.$(O) tblcmp.$(O) yylex.$(O)


ADD_COPT = -i=..$(SEP)include -pil

!include $(%ROOT)/mk/all.mk

TARGETS = $(DIR)$(SEP)$(PROJ).exe

$(DIR)$(SEP)$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 $(ADDFILES_CMD)
 $(SAY) Linking $@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
