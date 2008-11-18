# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = yacc
DESC = Parser generator
srcfiles = $(p)lr0$(e) $(p)allocate$(e) $(p)closure$(e) $(p)conflicts$(e) &
           $(p)derives$(e) $(p)files$(e) $(p)getargs$(e) $(p)gram$(e) $(p)lalr$(e) $(p)lex$(e) $(p)main$(e) &
           $(p)nullable$(e) $(p)output$(e) $(p)print$(e) $(p)reader$(e) $(p)reduce$(e) $(p)symtab$(e) &
           $(p)warshall$(e) $(p)getopt$(e) $(p)getopt1$(e)


!include $(%ROOT)/mk/tools.mk

TARGETS = inst

inst: $(MYDIR)bison.simple $(MYDIR)bison.hairy $(PATH)yacc.exe
 $(CP) $(MYDIR)bison.simple $(DEST)
 $(CP) $(MYDIR)bison.hairy $(DEST)
 $(CP) $(PATH)yacc.exe $(DEST)

ADD_COPT = -i=..$(SEP)include -dHAVE_CONFIG_H
STACKSIZE = 65536
