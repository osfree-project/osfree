#
# make macros
# for building OS/2 static libs
#

!ifndef __libsos2_mk__
!define __libsos2_mk__

CLEAN_ADD = *.c *.h

!include $(%ROOT)/tools/mk/libs.mk

ADD_COPT   =         -d__OS2__ -i=$(%WATCOM)$(SEP)h $(ADD_COPT)

!ifeq UNI2H 1

# generated uni2h headers

ADD_COPT   +=         -i=$(%ROOT)$(SEP)build$(SEP)include &
                      -i=$(%OS2TK)$(SEP)h &
                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)shared

!else

# use Watcom headers

ADD_COPT   +=         -d__OS2__ -i=$(%WATCOM)$(SEP)h$(SEP)os2

!endif

ADD_COPT   +=         -i=$(%ROOT)$(SEP)include &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3 &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pm

ADD_COPT   +=         -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)zlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)ojpeg &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)GDlib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)GL &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)lpng &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)jpeglib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)libtiff &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)gbm &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pdcurses &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)glib &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)acpitk &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)xwphelpers &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)lvmtk &
                      -bt=os2


!endif
