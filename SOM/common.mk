!ifdef HOSTBLD

DEST = host$(SEP)$(%HOST)$(SEP)$(DEST)

!ifeq %OS OS2
OPTS =
defs = $(%WATCOM)$(SEP)h$(SEP)os2
ADD_LINKOPT = lib clib3r
!endif
!ifeq %OS WIN32
OPTS = -br -dWIN32_LEAN_AND_MEAN
defs = $(%ROOT)SOM$(SEP)include -i=$(%WATCOM)$(SEP)h$(SEP)nt
ADD_LINKOPT = lib clbrdll
!endif
!ifeq %OS WIN64
OPTS = -br -dWIN32_LEAN_AND_MEAN
defs = $(%ROOT)SOM$(SEP)include -i=$(%WATCOM)$(SEP)h$(SEP)nt
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

defs = $(%ROOT)SOM$(SEP)include -i=$(%WATCOM)$(SEP)h$(SEP)os2 
ADD_LINKOPT = lib clib3r
OPTS = -bt=os2

!endif

C = r

!ifdef HOSTBLD
!include $(%ROOT)tools/mk/tools.mk
!else
!include $(%ROOT)tools/mk/appsos2.mk
!endif
!include $(%ROOT)SOM$(SEP)port$(SEP)somfree.mk

ADD_COPT = -dNDEBUG -i=$(defs) -i=$(PORT_BASE)include -bc -bm -dHAVE_CONFIG_H -dHAVE_CONFIG_HPP $(OPTS)
