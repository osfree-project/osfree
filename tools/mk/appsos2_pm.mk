!ifndef __apps_os2_pm__
!define __apps_os2_pm__

!ifeq UNI2H 1

#ADD_RCOPT     =       -i=$(%ROOT)$(SEP)build$(SEP)include 
#                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)os2 
#                      -i=$(%ROOT)$(SEP)build$(SEP)include$(SEP)shared 

# ADD_COPT     =        $(ADD_COPT) &
#                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pm

!else

#ADD_RCOPT     =       -i=$(%WATCOM)$(SEP)h &
#                      -i=$(%WATCOM)$(SEP)h$(SEP)os2

!endif

!include $(%ROOT)/tools/mk/appsos2_cmd.mk

!endif
