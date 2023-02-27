# $Id: makefile,v 1.3 2004/08/16 04:51:12 prokushev Exp $

PROJ = keystack
TRGT = $(PROJ).exe
DESC = KEYSTACK.EXE
srcfiles = $(p)keystack$(e)
STUB        = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
#UNI2H = 1
DEST = os2

!include $(%ROOT)tools/mk/appsos2_cmd.mk
!include $(MYDIR)..$(SEP)port$(SEP)4os2.mk

ADD_COPT = -i=$(PORT_BASE)h -ox

.c: $(PORT_BASE)c

.h: $(PORT_BASE)h
