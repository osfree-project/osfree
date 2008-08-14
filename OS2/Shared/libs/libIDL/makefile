# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs


!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

ADD_COPT = -i=..$(SEP)include -dLIBIDL_VERSION="0.8.10" -dHAVE_CPP_PIPE_STDIN=1 -dCPP_NOSTDINC="" -dCPP_PROGRAM="notsupported" -dYYTEXT_POINTER=1 -dSTDC_HEADERS=1 -dHAVE_STDDEF_H=1 -dHAVE_WCHAR_H=1 -dYYDEBUG=1 -dIDL_LIBRARY -dG_LOG_DOMAIN="libIDL"

!include $(%ROOT)/mk/all.mk

all: $(ROOT)$(SEP)lib$(SEP)libIDL.lib .SYMBOLIC

OBJS =  parser.$(O)                              &
        lexer.$(O)                               &
        ns.$(O)                                  &
        util.$(O)                                &
        ghash.$(O)                               &
        gnode.$(O)                               &
        gtree.$(O)                               &
        gatomic.$(O)                               &
        glist.$(O)                              &
        gslist.$(O)                              &
        gprimes.$(O)
#        gstring.$(O)                              &
#        gthread.$(O)                              &

# SED tool shiped with eComStation but not with plain OS/2. Seems SED
# need to be added to tools.
lexer.c: lexer.l parser.h
        $(DC) $@
        lex -t lexer.l | sed -e "s/yy/__IDL_/g" >$@

parser.h: stamp-parser
        $(RN) y.tab.h parser.h

parser.c: stamp-parser
        $(RN) y.tab.c parser.c

stamp-parser: parser.y
        $(DC) parser.h
        $(DC) parser.c
        yacc -y -d parser.y


$(ROOT)$(SEP)lib$(SEP)libIDL.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 $(LIB) $(LIBOPT) $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
