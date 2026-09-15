# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

DESCRIPTION = Lexical analyzer
SOURCES  = ccl dfa ecs gen main misc &
           nfa parse scan skel sym tblcmp yylex

ADD_COPT = -i=$(PATH) -i=$(MYDIR) -pil
STACKSIZE = 65536

TARGET_API = HOST
TARGET_BITS = 32

!include $(%ROOT)tools/mk/build.mk

#$(PATH)scan.obj:  $(PATH)scan.c

$(PATH)parse.obj: $(PATH)parse.c

$(PATH)parse.c: $(MYDIR)parse.y

#$(PATH)scan.c:  $(MYDIR)scan.c
# @$(CP) $^@ $[@

.c: $(PATH)
