# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = sed
TRGT = $(PROJ).exe
DESC = Sed language interpreter
srcfiles = $(p)sedcomp$(e) $(p)sedexec$(e)

#Notes:
# awk lex and yacc files not added to repository

ADD_COPT = -i=..$(SEP)include -za
STACKSIZE = 65536

!include $(%ROOT)tools/mk/tools.mk
