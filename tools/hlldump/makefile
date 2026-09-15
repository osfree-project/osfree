# $Id: makefile,v 1.3 2004/08/16 04:51:55 prokushev Exp $

DESCRIPTION = Debug information dump facility
TARGET_API = HOST
TARGET_BITS = 32

!include $(%ROOT)tools/mk/build.mk

ADD_COPT +=           -i=$(%OS2TK)$(SEP)h &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)shared

STACKSIZE = 65536
