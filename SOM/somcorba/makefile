
PROJ = somcorba
DEST = os2tk45$(SEP)som$(SEP)bin

!include $(%ROOT)SOM/common.mk

TARGETS = $(PATH)$(PROJ)
ext=

!ifneq %HOST linux
ext=.cmd
!else
!ifndef HOSTBLD
ext=.cmd
!endif
!endif

TARGETS = $(TARGETS)$(ext)

!ifdef HOSTBLD
!ifeq %OS WIN64
hostos=win32
!else
hostos=$(%HOST)
!endif
src=$(PORT_BASE)somcorb2$(SEP)$(hostos)$(SEP)$(PROJ)$(ext)
!else
src=$(PORT_BASE)somcorb2$(SEP)os2$(SEP)$(PROJ)$(ext)
!endif

$(TARGETS): $(src)
 @$(CP) $< $^@ $(BLACKHOLE)
