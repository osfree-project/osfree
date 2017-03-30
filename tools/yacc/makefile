# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = yacc
DESC = Parser generator
srcfiles = $(p)LR0$(e) $(p)allocate$(e) $(p)closure$(e) $(p)conflicts$(e) &
           $(p)derives$(e) $(p)files$(e) $(p)getargs$(e) $(p)gram$(e) $(p)lalr$(e) $(p)lex$(e) $(p)main$(e) &
           $(p)nullable$(e) $(p)output$(e) $(p)print$(e) $(p)reader$(e) $(p)reduce$(e) $(p)symtab$(e) &
           $(p)warshall$(e) $(p)getopt$(e) $(p)getopt1$(e)


!include $(%ROOT)/mk/tools.mk

TARGETS = inst

inst: $(MYDIR)bison.simple $(MYDIR)bison.hairy $(PATH)yacc.exe
 @$(CP) $(MYDIR)bison.simple $(DEST) $(BLACKHOLE)
 @$(CP) $(MYDIR)bison.hairy $(DEST) $(BLACKHOLE)
 @$(CP) $(PATH)yacc.exe $(DEST) $(BLACKHOLE)

ADD_COPT = -i=..$(SEP)include -dHAVE_CONFIG_H
STACKSIZE = 65536
