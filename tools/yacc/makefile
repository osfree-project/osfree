# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = yacc
DESC = Parser generator
srcfiles = $(p)lr0$(e) $(p)allocate$(e) $(p)closure$(e) $(p)conflicts$(e) &
           $(p)derives$(e) $(p)files$(e) $(p)getargs$(e) $(p)gram$(e) $(p)lalr$(e) $(p)lex$(e) $(p)main$(e) &
           $(p)nullable$(e) $(p)output$(e) $(p)print$(e) $(p)reader$(e) $(p)reduce$(e) $(p)symtab$(e) &
           $(p)warshall$(e) $(p)getopt$(e) $(p)getopt1$(e)
STACKSIZE = 65536

!include $(%ROOT)/mk/tools.mk

ADD_COPT = -i=..$(SEP)include -dHAVE_CONFIG_H
