!ifndef __apps_os2_fpm__
!define __apps_os2_fpm__

!include $(%ROOT)/mk/appsos2_pm.mk

ADD_COPT     =        $(ADD_COPT) &
                      -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pm

!endif
