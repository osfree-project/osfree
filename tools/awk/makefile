# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

DESCRIPTION = AWK language interpreter
SOURCES  = b main parse proctab tran lib run lex &
           ytab missing95

ADD_COPT = -i=..$(SEP)include 
STACKSIZE = 65536

!include $(%ROOT)tools/mk/build.mk
