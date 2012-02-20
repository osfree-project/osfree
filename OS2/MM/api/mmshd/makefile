#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = mmshd
DESC     = MM Shadow
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = MMShadowCClassData      MMPARTS.250,  &
          MMShadowClassData       MMPARTS.251,  &
          MMShadowNewClass        MMPARTS.252,  &
          M_MMShadowCClassData    MMPARTS.253,  &
          M_MMShadowClassData     MMPARTS.254,  &
          M_MMShadowNewClass      MMPARTS.255
EXPORTS = MMShadowCClassData.2,   &
          MMShadowClassData.3,    &
          MMShadowNewClass.4,     &
          M_MMShadowCClassData.5, &
          M_MMShadowClassData.6,  &
          M_MMShadowNewClass.7

DEST    = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
