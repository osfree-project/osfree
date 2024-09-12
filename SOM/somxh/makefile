
PROJ = somxh
DEST = os2tk45$(SEP)som$(SEP)bin

!include $(%ROOT)SOM/common.mk

TARGETS = $(PATH)$(PROJ)

!ifneq %HOST linux
TARGETS = $(TARGETS).cmd
!endif

!ifdef HOSTBLD
!ifeq %OS WIN64
hostos=win32
!else
hostos=$(%HOST)
!endif
src=$(PORT_BASE)$(PROJ)$(SEP)$(hostos)$(SEP)$(PROJ)
!ifneq %HOST linux
src=$(src).cmd
!endif
!else
src=$(PORT_BASE)$(PROJ)$(SEP)os2$(SEP)$(PROJ).cmd
!endif

$(TARGETS): $(src)
 @$(CP) $< $^@ $(BLACKHOLE)

