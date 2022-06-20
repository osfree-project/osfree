# $Id: makefile,v 1.3 2004/08/16 04:51:55 prokushev Exp $

PROJ = hlldump
TRGT = $(PROJ).exe
DESC = Debug information dump facility
srcfiles = $(p)hll$(e) $(p)hlldirectory$(e) $(p)exe$(e) $(p)hllmodule$(e) $(p)hllpublics$(e) &
           $(p)hlllines$(e) $(p)hllsymbols$(e) $(p)hlltypes$(e)

!include $(%ROOT)/mk/tools.mk

ADD_COPT +=           -i=$(%OS2TK)$(SEP)h &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)shared

STACKSIZE = 65536
