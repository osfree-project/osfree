#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = nampipes.dll
DESC     = Named Pipes API
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = DOSMAKENMPIPE       DOSCALLS.399, &
          DOSQNMPIPEINFO      DOSCALLS.400, &
          DOSCONNECTNMPIPE    DOSCALLS.401, &
          DOSDISCONNECTNMPIPE DOSCALLS.402, &
          DOSQNMPHANDSTATE    DOSCALLS.403, &
          DOSSETNMPHANDSTATE  DOSCALLS.404, &
          DOSPEEKNMPIPE       DOSCALLS.405, &
          DOSWAITNMPIPE       DOSCALLS.406, &
          DOSTRANSACTNMPIPE   DOSCALLS.407, &
          DOSCALLNMPIPE       DOSCALLS.408, &
          DOSRAWREADNMPIPE    DOSCALLS.409, &
          DOSRAWWRITENMPIPE   DOSCALLS.410, &
          DOSSETNMPIPESEM     DOSCALLS.411, &
          DOSQNMPIPESEMSTATE  DOSCALLS.412

EXPORTS = DOSMAKENMPIPE       .1  RESIDENT, &
          DOSQNMPIPEINFO      .2  RESIDENT, &
          DOSCONNECTNMPIPE    .3  RESIDENT, &
          DOSDISCONNECTNMPIPE .4  RESIDENT, &
          DOSQNMPHANDSTATE    .5  RESIDENT, &
          DOSSETNMPHANDSTATE  .6  RESIDENT, &
          DOSPEEKNMPIPE       .7  RESIDENT, &
          DOSWAITNMPIPE       .8  RESIDENT, &
          DOSTRANSACTNMPIPE   .9  RESIDENT, &
          DOSCALLNMPIPE       .10 RESIDENT, &
          DOSRAWREADNMPIPE    .11 RESIDENT, &
          DOSRAWWRITENMPIPE   .12 RESIDENT, &
          DOSSETNMPIPESEM     .13 RESIDENT, &
          DOSQNMPIPESEMSTATE  .14 RESIDENT

DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk




