#
# make macros
# for building OS/2 static libs
#

!ifndef __libsos2_mk__
!define __libsos2_mk__

CLEAN_ADD = *.c *.h
ADD_COPT =            $(ADD_COPT) &
                      -i=$(ROOT)$(SEP)build$(SEP)include &
                      -i=$(ROOT)$(SEP)build$(SEP)include$(SEP)os2 &
                      -i=$(ROOT)$(SEP)include$(SEP)os3 &
                      -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)pm &
                      -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)GDlib &
                      -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)zlib &
                      -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)lpng &
                      -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)jpeglib &
                      -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)libtiff &
                      -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)ojpeg &
                      -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)gbm &
                      -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)pdcurses &
                      -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)glib &
                      -bt=os2

!include $(%ROOT)/mk/libs.mk

!endif
