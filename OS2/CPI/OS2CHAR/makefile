#
# A Makefile for os2char.dll
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
IMPORTS = Dos16SMGetmem   DOSCALLS.682, &
          Dos16SMFreemem  DOSCALLS.683, &
          Dos16SMGetSGCB  DOSCALLS.684, &
          Dos16SMInitSGCB DOSCALLS.685, &
          SMQueueRamSem   DOSCALLS.858, &
          SMArray         DOSCALLS.859, &
          SMPIDArray      DOSCALLS.860, &
          SMArraySize     DOSCALLS.862, &
          SMGINSTANCE     DOSCALLS.864, &
          SMInitialized   DOSCALLS.861, &
          BVHINSTANCE     DOSCALLS.865, &
          BVSGLOBAL       DOSCALLS.863, &
          OS2BaseInit     DOSCALLS.718

EXPORTS = Dos16SMGetmem   .1, &
          Dos16SMFreemem  .2, &
          Dos16SMGetSGCB  .3, &
          Dos16SMInitSGCB .4, &
          SMQueueRamSem   .5, &
          SMArray         .6, &
          SMPIDArray      .7, &
          SMArraySize     .8, &
          SMGINSTANCE     .9, &
          SMInitialized   .99, &
          BVHINSTANCE     .148, &
          BVSGLOBAL       .149, &
          OS2BaseInit     .152
DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
