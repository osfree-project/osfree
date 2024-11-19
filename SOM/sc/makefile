
!ifdef HOSTBLD
!ifeq %OS WIN32
PROJ = somc
!else ifeq %OS WIN64
PROJ = somc
!else
PROJ = sc
!endif
!else
PROJ = sc
!endif
TRGT = $(PROJ).exe
srcfiles = $(p)sc$(e)
DEST = os2tk45$(SEP)som$(SEP)bin

!include $(%ROOT)SOM/common.mk

.c: $(PORT_BASE)sc$(SEP)src

#.h: $(SRC)contrib$(SEP)include

#.$(O): $(PATH)
