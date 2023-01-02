#
# make macros
# for building DOS static libs
#

!ifndef __libsdos_mk__
!define __libsdos_mk__

32_BITS = 0
ADD_COPT =            $(ADD_COPT) &
                      -bt=dos
#                     -i=$(ROOT)$(SEP)build$(SEP)include &
#                     -i=$(ROOT)$(SEP)build$(SEP)include$(SEP)dos &

!include $(%ROOT)/tools/mk/libs.mk

!endif
