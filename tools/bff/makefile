# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

DESCRIPTION = Binary File Firmat parser
SOURCES = bff bff_lex
TARGET_API = HOST
TARGET_BITS = 32

!include $(%ROOT)tools/mk/build.mk

ADD_COPT = -i=$(PATH) -i=$(MYDIR) -pil
STACKSIZE = 65536

$(PATH)bff.obj: $(PATH)bff.c

$(PATH)bff_lex.obj: $(PATH)bff_lex.c
