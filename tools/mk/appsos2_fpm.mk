!ifndef __apps_os2_fpm__
!define __apps_os2_fpm__

!include $(%ROOT)/tools/mk/appsos2_pm.mk

ADD_COPT     += $(ADD_COPT) &
               -3r -i=$(%ROOT)$(SEP)include$(SEP)os3$(SEP)pm

ADD_LINKOPT  += lib $(%WATCOM)$(SEP)lib386$(SEP)os2$(SEP)plib3r.lib
CXX          = 1

!endif
