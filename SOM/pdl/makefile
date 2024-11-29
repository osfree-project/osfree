
PROJ = pdl
TRGT = $(PROJ).exe
DESC = pdl
srcfiles = $(p)pdl$(e)

!ifdef HOSTBLD

DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)bin

!ifeq %OS OS2
OPTS =
defs = $(%WATCOM)$(SEP)h$(SEP)os2
ADD_LINKOPT = lib clib3r
!endif
!ifeq %OS WIN32
OPTS = -br -dWIN32_LEAN_AND_MEAN
defs = $(%WATCOM)$(SEP)h$(SEP)nt
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS WIN64
OPTS = -br -dWIN32_LEAN_AND_MEAN
defs = $(%WATCOM)$(SEP)h$(SEP)nt
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS DOS
OPTS =
defs = $(%WATCOM)$(SEP)h$(SEP)dos
ADD_LINKOPT = lib clib3r
!endif
!ifeq %OS LINUX
OPTS =
defs = $(%WATCOM)$(SEP)lh
ADD_LINKOPT = lib clib3r
!endif

!else

DEST = os2tk45$(SEP)som$(SEP)bin

defs = $(%WATCOM)$(SEP)h$(SEP)os2
ADD_LINKOPT = lib clib3r

!endif

C = r

!ifdef HOSTBLD
!include $(%ROOT)tools/mk/tools.mk
!else
!include $(%ROOT)tools/mk/appsos2.mk
!endif
!include $(%ROOT)SOM$(SEP)port$(SEP)somfree.mk

ADD_COPT = -dNDEBUG -i=$(defs) -i=$(PORT_BASE)include -bc -bm -dHAVE_CONFIG_H $(OPTS)

.c: $(PORT_BASE)pdl$(SEP)src
