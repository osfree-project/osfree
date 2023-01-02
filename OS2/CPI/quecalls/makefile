#
# A Makefile for quecalls.dll
# (c) osFree project,
# prokushev, date
#

PROJ     = quecalls
TRGT     = $(PROJ).dll
DESC     = Queues API
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = &
          DOSREADQUEUE    DOSCALLS.674, &
          DOSPURGEQUEUE   DOSCALLS.675, &
          DOSCLOSEQUEUE   DOSCALLS.676, &
          DOSQUERYQUEUE   DOSCALLS.677, &
          DOSPEEKQUEUE    DOSCALLS.678, &
          DOSWRITEQUEUE   DOSCALLS.679, &
          DOSOPENQUEUE    DOSCALLS.680, &
          DOSCREATEQUEUE  DOSCALLS.681, &
          DosReadQueue2   DOSCALLS.661, &
          DosPurgeQueue2  DOSCALLS.662, &
          DosCloseQueue2  DOSCALLS.663, &
          DosQueryQueue2  DOSCALLS.664, &
          DosPeekQueue2   DOSCALLS.665, &
          DosWriteQueue2  DOSCALLS.666, &
          DosOpenQueue2   DOSCALLS.667, &
          DosCreateQueue2 DOSCALLS.668
EXPORTS = &
         DOSREADQUEUE.1,    &
          DOSPURGEQUEUE.2,   &
          DOSCLOSEQUEUE.3,   &
          DOSQUERYQUEUE.4,   &
          DOSPEEKQUEUE.5,    &
          DOSWRITEQUEUE.6,   &
          DOSOPENQUEUE.7,    &
          DOSCREATEQUEUE.8,  &
          DosReadQueue2.9,   &
          DosPurgeQueue2.10, &
          DosCloseQueue2.11, &
          DosQueryQueue2.12, &
          DosPeekQueue2.13,  &
          DosWriteQueue2.14, &
          DosOpenQueue2.15,  &
          DosCreateQueue2.16

DEST    = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
