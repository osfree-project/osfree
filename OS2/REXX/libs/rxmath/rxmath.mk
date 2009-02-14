#
# A makefile for rxmath.dll
#

DBG = 1
#DBG = 0

!ifeq DBG 1
OPT   = -d3 -of -od
!else
OPT   = -d0 -oteanx
!endif

DESC        = REXX math API library
ADD_COPT    = -i=$(MYDIR).. -bd -bm -wx -s -mf $(OPT) -6s -fp6 -bd
ADD_LINKOPT = lib rexx.lib
OPTIONS     = manyautodata
DLL         = 1
DLLOPT      = initinstance terminstance
EXPORTS     = MATHLOADFUNCS.1 = mathloadfuncs

DEST         = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
