!ifndef __apps_os2_cmd__
!define __apps_os2_cmd__

!include $(%ROOT)/tools/mk/appsos2.mk

!ifneq NOLIBS 1
ADD_LINKOPT += lib $(BLD)lib$(SEP)cmd_shared.lib 
!endif

!endif
