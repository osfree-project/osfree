# $Id: makefile,v 1.3 2004/08/16 04:51:12 prokushev Exp $

PROJ = option2
DESC = OPTION2.EXE
srcfiles    = $(p)option2$(e) $(p)os2cfg$(e) $(p)os2nb$(e) $(p)..$(SEP)shared$(SEP)istring$(e)
ADD_COPT    = -i=$(MYDIR)..$(SEP)include -ox
STUB        = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
RESOURCE    = $(PATH)option2.res
ADD_RCOPT   = -i=..$(SEP)include

!include $(%ROOT)/mk/appsos2.mk
