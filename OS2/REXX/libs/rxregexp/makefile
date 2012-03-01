#
# A makefile for rxregexp
# author valerius, 2009 Feb 14
#

PROJ  = rxregexp
DESC  = Regular expression support for REXX

DBG   = 1
#DBG   = 0

!ifeq DBG 1
OPT   = -d3 -of -od
!else
OPT   = -d0 -oteanx
!endif

srcfiles    = $(p)rxregexp$(e) $(p)automaton$(e) $(p)dblqueue$(e)

ADD_COPT    = -i=$(MYDIR) -i=$(MYDIR)oorxinc -bd -bm -wx -xs -s -mf $(OPT) -6s -fp6 -bd
ADD_LINKOPT = lib rexx.lib
OPTIONS     = manyautodata
DLL         = 1
DLLOPT      = initinstance terminstance
EXPORTS     = &
  RegExp_Init, &
  RegExp_Uninit, &
  RegExp_Parse, &
  RegExp_Pos, &
  RegExp_Match

!include $(%ROOT)/mk/appsos2.mk
 