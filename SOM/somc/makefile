
PROJ = somc
TRGT = $(PROJ).dll
srcfiles = $(p)somc$(e) &
	$(p)symtab$(e) &
	$(p)emitlib$(e) &
	$(p)modifiers$(e) &
	$(p)smlib$(e)

DEST = os2tk45$(SEP)som$(SEP)lib

DLL=1

!include $(%ROOT)SOM/common.mk

!ifdef HOSTBLD
OS2TK = $(%OS2TK)
!else
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45
!endif

ADD_COPT = $(ADD_COPT) -i=$(OS2TK)$(SEP)som$(SEP)include -i=$(PORT_BASE)somc$(SEP)include

.c: $(PORT_BASE)somc$(SEP)src

.h: $(PORT_BASE)somc$(SEP)include
