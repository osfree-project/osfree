
PROJ = somstars
DEST = os2tk45$(SEP)som$(SEP)bin

!include $(%ROOT)SOM/common.mk

TARGETS = $(PATH)$(PROJ).cmd

!ifdef HOSTBLD
!ifeq %OS WIN64
hostos=win32
!else
hostos=$(%OS)
!endif
src=$(PORT_BASE)$(PROJ)$(SEP)$(hostos)$(SEP)$(PROJ).cmd
!else
src=$(PORT_BASE)$(PROJ)$(SEP)os2$(SEP)$(PROJ).cmd
!endif

$(PATH)$(PROJ).cmd: $(src)
 @$(CP) $< $^@ $(BLACKHOLE)

