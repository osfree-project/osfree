#
# A Makefile for utlapi.dll
# (c) osFree project,
# author, date
#

PROJ     = utlapi
TRGT     = $(PROJ).dll
DESC     = osFree Utilities API
srcfiles = $(p)librectangles$(e)
ADD_COPT = -i=..$(SEP)..$(SEP)include$(SEP)os3
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
EXPORTS = RectCopy.1, RectSet.2, RectIsEmpty.3, &
          RectEqual.4, RectSetEmpty.5, RectOffset.6, &
          RectInflate.7, RectPtIn.8, RectIntersect.9, &
          RectUnion.10, RectSubtract.11, RectMakeRect.12, &
          RectMakePoints.13
DEST    = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk

