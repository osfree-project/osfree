# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = lex
DESC = Lexical analyzer
srcfiles = $(p)ccl$(e) $(p)dfa$(e) $(p)ecs$(e) $(p)gen$(e) $(p)main$(e) $(p)misc$(e) &
           $(p)nfa$(e) $(p)parse$(e) $(p)scan$(e) $(p)skel$(e) $(p)sym$(e) $(p)tblcmp$(e) $(p)yylex$(e)

!include $(%ROOT)/mk/tools.mk

ADD_COPT = -i=$(PATH) -i=$(MYDIR) -pil
STACKSIZE = 65536

$(PATH)scan.obj:  $(PATH)scan.c

$(PATH)parse.obj: $(PATH)parse.c

$(PATH)scan.c:  $(MYDIR)scan.c
 $(CP) $^@ $[@
