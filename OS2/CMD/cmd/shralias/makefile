# $Id: makefile,v 1.3 2004/08/16 04:51:13 prokushev Exp $

PROJ = shralias
TRGT = $(PROJ).exe
DESC = SHRALIAS.EXE
srcfiles = $(p)shralias$(e)
ADD_COPT = -i=$(MYDIR)..$(SEP)contrib$(SEP)h -ox
STUB        = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
UNI2H = 1
DEST = os2

!include $(%ROOT)tools/mk/appsos2_cmd.mk

.c: $(MYDIR)..$(SEP)contrib$(SEP)c

.h: $(MYDIR)..$(SEP)contrib$(SEP)h
