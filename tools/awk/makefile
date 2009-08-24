# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = awk
DESC = AWK language interpreter
srcfiles = $(p)b$(e) $(p)main$(e) $(p)parse$(e) $(p)proctab$(e) $(p)tran$(e) $(p)lib$(e) $(p)run$(e) $(p)lex$(e) &
           $(p)ytab$(e) $(p)missing95$(e)

!include $(%ROOT)/mk/tools.mk

ADD_COPT = -i=..$(SEP)include
TARGETS = $(PATH)$(PROJ).exe
STACKSIZE = 65536
