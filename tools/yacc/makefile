# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

32_BITS  = 1

DIR = $(ROOT)$(SEP)tools$(SEP)bin
PROJ = yacc
DESC = Parser generator
OBJS = LR0.$(O) allocate.$(O) closure.$(O) conflicts.$(O) &
derives.$(O) files.$(O) getargs.$(O) gram.$(O) lalr.$(O) lex.$(O) main.$(O) &
nullable.$(O) output.$(O) print.$(O) reader.$(O) reduce.$(O) symtab.$(O) &
warshall.$(O) getopt.$(O) getopt1.$(O)

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
