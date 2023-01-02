# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = bff
DESC = Binary File Firmat parser
srcfiles = $(p)bff$(e) $(p)bff_lex$(e) 

!include $(%ROOT)tools/mk/tools.mk

ADD_COPT = -i=$(PATH) -i=$(MYDIR) -pil
STACKSIZE = 65536

$(PATH)bff.obj: $(PATH)bff.c

$(PATH)bff_lex.obj: $(PATH)bff_lex.c

