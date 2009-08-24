# $Id: makefile,v 1.2 2000/04/24 00:22:23 bird Exp $
#
#  JFS lib directory makefile - makes kee.lib
#

ROOT= $(MAKEDIR:\lib=)
!include $(ROOT)\header.mak


all: kee.lib fshelper.lib


kee.lib: kee.def
    $(IMPLIB) $@ $**

fshelper.lib: fshelper.def
    $(IMPLIB) $@ $**

clean:
    -del kee.lib
    -del fshelper.lib

dep:

