/*  REXX API init DLL
 *  (c) osFree project, 2012 Feb 27
 *  author: valerius
 */

#include "rexxinit.h"

/* Semaphores         */
char rexxApiMtx[] = "\\SEM32\\REXXAPI.SEM";
char rexxMemEvt[] = "\\SEM32\\REXXMEM.SEM";
/* Queue              */
char rexxQue[]    = "\\QUEUES\\RXQUEUE";
/* REXX API DLL name  */
char pszModName[] = "REXXAPI";

HEV   evq      = 0;
ULONG cb       = 0;
ULONG initflag = 1;

/* DLL init function  */
APIRET APIENTRY dllinit (ULONG param)
{
  RxApiInit(param);

  return 1;
}

/* REXX API init      */
APIRET APIENTRY RxApiInit (ULONG param)
{
  APIRET rc;
  TID    tid;

  if (!initflag)
    return NO_ERROR;

  rc = DosCreateThread(&tid, (PFNTHREAD)&RxInitThread, 0, 0, THREAD_STACK);
  initflag = 0;

  return rc;
}

/* worker thread      */
void APIENTRY RxInitThread (void *param)
{
  BYTE          priority = 0;
  ULONG         cbData   = 0;
  REQUESTDATA   request  = {0};
  buf_t         buf      = {0, 0, 0, 0, 0};
  buf_t         *pb      = &buf;
  APIRET        rc;
  USHORT        a;

  DosCreateMutexSem(rexxApiMtx, &pb->hmtxa, 0, 0);
  DosCreateEventSem(rexxMemEvt, &pb->hev,  0, 0);
  DosCreateQueue(&pb->hq, 0, rexxQue);
  
  for (;;) 
  {
    DosReadQueue(pb->hq, &request, &cbData, (void **)&pb, 0, 0, &priority, evq);

    a = (USHORT)request.ulData; 

    if (a == CMD_QUIT) // 0xffff
      break;

    switch (a)
    {
      case CMD_INIT:   // 0xfff8
        DosSubSetMem(pb, DOSSUB_INIT | DOSSUB_SPARSE_OBJ, cb);
        DosPostEventSem(pb->hev);
        continue;
      case CMD_DONE:   // 0xfff9
        DosSubUnsetMem(pb);
        DosFreeMem(pb);
        continue;
      case CMD_FREE:   // 0xfffa
        DosFreeMem(pb);
        continue;
      case CMD_CLOSE:  // 0xfffb
        DosCloseMutexSem(pb->hmtx);
        DosPostEventSem(pb->hev);
        DosCloseEventSem(pb->hev);
        continue;
      case CMD_OPEN:   // 0xfffc
        DosOpenMutexSem(0, &pb->hmtx);
        DosOpenEventSem(0, &pb->hev);
        DosPostEventSem(pb->hev);
        continue;
      case CMD_LOAD:   // 0xfffd
        if (pb->hmod)
          continue;
        DosLoadModule(0, 0, pszModName, &pb->hmod);
        continue;
      default:
        continue;
    }
    
    break;
  }
  
  initflag = 1;

  DosCloseQueue(pb->hq);
  DosFreeModule(pb->hmod);
  DosCloseMutexSem(pb->hmtxa);
}
