#
# A makefile for rxmath.dll
#

DBG = 0
#DBG = 1

!ifeq DBG 1
OPT   = -d3 -of -od
!else
OPT   = -d0 -oteanx
!endif

DESC        = REXX math API library
ADD_COPT    = -dDYNAMIC -i=$(MYDIR).. -bm -wx -s -mf $(OPT) -fp6 -6s
ADD_LINKOPT = lib clib3s.lib,math3s.lib # rexx.lib
OPTIONS     = manyautodata
DLL         = 1
DLLOPT      = initinstance terminstance
UNI2H       = 1
C = s

!include $(%ROOT)/mk/appsos2_cmd.mk
