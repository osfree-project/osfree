# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

DESCRIPTION = AWK language interpreter
SOURCES  = proctab b main parse tran lib run lex missing95
DIRS = awk maketab
LIBS = awk

ADD_COPT = -i=..$(SEP)include -I$(PATH)awk

STACKSIZE = 65536

#LICENSE_CHECK = 1

!include $(%ROOT)tools/mk/build.mk

$(PATH)proctab.c: $(PATH)awk$(SEP)ytab.h
  $(CD) $(PATH)awk && maketab > $(PATH)proctab.c && $(CD) $(CWD)

.c: $(PATH)

.h: $(PATH)

