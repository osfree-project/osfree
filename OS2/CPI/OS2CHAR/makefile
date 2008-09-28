#
# A Makefile for sesmgr.dll
# (c) osFree project,
# author, date
#

PROJ        = os2char
DESC        = os2char API
OBJS        = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT    = -i=..\include
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = Undocumented1   DOSCALLS.682, &
          Undocumented2   DOSCALLS.683, &
          Undocumented3   DOSCALLS.684, &
          Undocumented4   DOSCALLS.685, &
          Undocumented5   DOSCALLS.858, &
          Undocumented6   DOSCALLS.859, &
          Undocumented7   DOSCALLS.860, &
          Undocumented8   DOSCALLS.862, &
          Undocumented9   DOSCALLS.864, &
          Undocumented99  DOSCALLS.861, &
          Undocumented148 DOSCALLS.865, &
          Undocumented149 DOSCALLS.863, &
          Undocumented152 DOSCALLS.718
EXPORTS = Undocumented1   .1, &
          Undocumented2   .2, &
          Undocumented3   .3, &
          Undocumented4   .4, &
          Undocumented5   .5, &
          Undocumented6   .6, &
          Undocumented7   .7, &
          Undocumented8   .8, &
          Undocumented9   .9, &
          Undocumented99  .99, &
          Undocumented148 .148, &
          Undocumented149 .149, &
          Undocumented152 .152
DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
