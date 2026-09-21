# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#

TARGET_API      = OS2
TARGET_CLASS    = LIBRARY
TARGET_SUBCLASS = STATIC
ADD_COPT        = -i=$(MYDIR)..$(SEP)..$(SEP)..$(SEP)include

!include $(%ROOT)tools/mk/build.mk

.c: $(MYDIR)..$(SEP)..$(SEP)..$(SEP)shared$(SEP)ccl
