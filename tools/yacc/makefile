# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = yacc
DESC = Parser generator
srcfiles = $(p)LR0$(e) $(p)allocate$(e) $(p)closure$(e) $(p)conflicts$(e) &
           $(p)derives$(e) $(p)files$(e) $(p)getargs$(e) $(p)gram$(e) $(p)lalr$(e) $(p)lex$(e) $(p)main$(e) &
           $(p)nullable$(e) $(p)output$(e) $(p)print$(e) $(p)reader$(e) $(p)reduce$(e) $(p)symtab$(e) &
           $(p)warshall$(e) $(p)getopt$(e) $(p)getopt1$(e)

!ifdef UNIX
ALIAS = mktemp=mkstemp_
!endif

!include $(%ROOT)/mk/tools.mk

TARGETS = inst $(PATH)yacc
# $(PATH)yacc
inst: $(MYDIR)bison.simple $(MYDIR)bison.hairy $(PATH)yacc.exe
 $(CP) $(MYDIR)bison.simple $(DEST)
 $(CP) $(MYDIR)bison.hairy $(DEST)
 $(CP) $(PATH)yacc.exe $(DEST)
 $(CP) $(PATH)yacc.exe $(DEST)yacc

# The variable ADD_COPT is overwritten in mk/tools.mk when under Linux and I had 
# some problems with using it.
# The variable DEFINES is added to tools.mk

ADD_COPT = -i=..$(SEP)include -dHAVE_CONFIG_H
DEFINES = -i=..$(SEP)include -dHAVE_CONFIG_H
STACKSIZE = 65536
