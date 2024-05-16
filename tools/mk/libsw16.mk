#
# make macros
# for building DOS static libs
#

!ifndef __libsw16_mk__
!define __libsw16_mk__

32_BITS = 0
ADD_COPT =            $(ADD_COPT) &
                      -bt=windows &
						-i=. -i=$(WATCOM)$(SEP)h$(SEP)win $(ADD_COPT)

!include $(%ROOT)/tools/mk/libs.mk

!endif
