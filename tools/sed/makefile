# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = SED
TRGT = $(PROJ).exe
DESC = SED language interpreter
srcfiles = $(p)sedcomp$(e) $(p)sedexec$(e)

!include $(%ROOT)/mk/tools.mk

#Notes:
# awk lex and yacc files not added to repository

ADD_COPT = -i=..$(SEP)include
STACKSIZE = 65536
