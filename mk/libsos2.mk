#
# make macros
# for building OS/2 static libs
#

!ifndef __libsos2_mk__
!define __libsos2_mk__

CLEAN_ADD = *.c *.h

!ifeq UNI2H 1
# generated uni2h headers

ADD_COPT   =          $(ADD_COPT) -i=$(%WATCOM)$(SEP)h -d__OS2__ &
                      -i=$(%ROOT)$(SEP)build$(SEP)include &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)os2 &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)shared

ADD_LINKOPT = option nod lib $(BLD)lib$(SEP)sub32.lib,$(BLD)lib$(SEP)clibext.lib,$(BLD)lib$(SEP)os2386.lib &
              lib $(%WATCOM)$(SEP)lib386$(SEP)math387r.lib,$(%WATCOM)$(SEP)lib386$(SEP)os2$(SEP)clib3r.lib

!else
# use Watcom headers

ADD_COPT   =          $(ADD_COPT) -d__OS2__ &
                      -i=$(%WATCOM)$(SEP)h$(SEP)os2

!endif

ADD_COPT   =          $(ADD_COPT) &
                      -i=$(%ROOT)$(SEP)include &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3 &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pm

ADD_COPT   =          $(ADD_COPT) &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)zlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)ojpeg &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)GDlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)GL &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)lpng &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)jpeglib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)libtiff &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)gbm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pdcurses &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)glib &
                      -bt=os2

!include $(%ROOT)/mk/libs.mk

!endif
