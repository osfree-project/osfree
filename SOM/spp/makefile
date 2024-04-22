
PROJ = spp
TRGT = $(PROJ).exe
srcfiles = $(p)spp$(e)
DEST = os2tk45$(SEP)som$(SEP)bin

!include $(%ROOT)SOM/common.mk

!ifdef HOSTBLD
OS2TK = $(%OS2TK)$(SEP)
!else
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45$(SEP)
!endif

ADD_LINKOPT = lib $(OS2TK)som$(SEP)lib$(SEP)somtk

ADD_COPT = $(ADD_COPT) -i=$(OS2TK)$(SEP)som$(SEP)include 

.c: $(PORT_BASE)spp$(SEP)src



