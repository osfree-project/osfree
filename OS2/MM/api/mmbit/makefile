#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = mmbit
DESC     = Bitmap MM Class
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = MMBitmapCClassData      MMPARTS.98,  &
	  MMBitmapClassData       MMPARTS.99,  &
	  MMBitmapNewClass        MMPARTS.100, &
	  M_MMBitmapCClassData    MMPARTS.101, &
	  M_MMBitmapClassData     MMPARTS.102, &
	  M_MMBitmapNewClass      MMPARTS.103
EXPORTS = MMBitmapCClassData.2,     &
	  MMBitmapClassData.3,      &
	  MMBitmapNewClass.4,       &
	  M_MMBitmapCClassData.5,   &
	  M_MMBitmapClassData.6,    &
	  M_MMBitmapNewClass.7

DEST    = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
