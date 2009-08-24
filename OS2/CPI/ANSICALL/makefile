#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = ansicall
DESC     = ANSI escape sequence API
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = ANSIINJECT DOSCALLS.848, &
          ANSIKEYDEF DOSCALLS.849, &
          ANSIINTERP DOSCALLS.850
EXPORTS = ANSIINJECT.1 RESIDENT, &
          ANSIKEYDEF.2 RESIDENT, &
          ANSIINTERP.3 RESIDENT

DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk


