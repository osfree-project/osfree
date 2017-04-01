# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = somcpp
TRGT = $(PROJ).exe
DESC = SOM Compiler Preprocessor
srcfiles = $(p)main$(e) $(p)directive$(e) $(p)eval$(e) $(p)expand$(e) &
           $(p)support$(e) $(p)system$(e) $(p)mbchar$(e)

!include $(%ROOT)/mk/tools.mk

#Notes:
# awk lex and yacc files not added to repository

ADD_COPT = -i=..$(SEP)include -sg -d2
STACKSIZE = 6553500
