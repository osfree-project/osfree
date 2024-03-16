
PROJ = somenv
DEST = os2tk45$(SEP)som$(SEP)bin

!include $(%ROOT)SOM/common.mk

TARGETS = $(PATH)$(PROJ).cmd

!ifdef HOSTBLD
src=$(PORT_BASE)$(PROJ)$(SEP)$(%OS)$(SEP)$(PROJ).cmd
!else
src=$(PORT_BASE)$(PROJ)$(SEP)os2$(SEP)$(PROJ).cmd
!endif

$(PATH)$(PROJ).cmd: $(src)
 @$(CP) $< $^@ $(BLACKHOLE)

