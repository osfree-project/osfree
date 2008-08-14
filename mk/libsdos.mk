#
# make macros
# for building OS/2 static libs
#

32_BITS = 0
ADD_COPT =            $(ADD_COPT) &
                      #-i=$(ROOT)$(SEP)include$(SEP)os3 &
                      -i=$(ROOT)$(SEP)include$(SEP)os3\dos &
                      -bt=dos

!include $(%ROOT)/mk/libs.mk

q: .SYMBOLIC
 $(SAY) $(CC)
