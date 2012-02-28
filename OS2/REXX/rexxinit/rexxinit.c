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
  ULONG         unused1;
  ULONG         cbData   = 0;
  REQUESTDATA   request  = {0, 0};
  HEV           hev;
  HQUEUE        hq;
  HMTX          hmtx;
  HMODULE       hmod;
  buf_t         *pbuf;
  APIRET        rc;
  USHORT        cmd;

  DosCreateMutexSem(rexxApiMtx, &hmtx, 0, 0);
  DosCreateEventSem(rexxMemEvt, &hev,  0, 0);
  DosCreateQueue(&hq, 0, rexxQue);
  
  for (;;) 
  {
    DosReadQueue(hq, &request, &cbData, (void **)&pbuf, 0, 0, &priority, evq);

    cmd = (USHORT)request.ulData; 

    if (cmd == CMD_QUIT) // 0xffff
      break;

    switch (cmd)
    {
      case CMD_INIT:   // 0xfff8
        DosSubSetMem(pbuf, DOSSUB_INIT | DOSSUB_SPARSE_OBJ, cb);
        DosPostEventSem(hev);
        continue;
      case CMD_DONE:   // 0xfff9
        DosSubUnsetMem(pbuf);
        DosFreeMem(pbuf);
        continue;
      case CMD_FREE:   // 0xfffa
        DosFreeMem(pbuf);
        continue;
      case CMD_CLOSE:  // 0xfffb
        DosCloseMutexSem(pbuf->hmtx);
        DosPostEventSem(pbuf->hev);
        DosCloseEventSem(pbuf->hev);
        continue;
      case CMD_OPEN:   // 0xfffc
        DosOpenMutexSem(0, &pbuf->hmtx);
        DosOpenEventSem(0, &pbuf->hev);
        DosPostEventSem(pbuf->hev);
        continue;
      case CMD_LOAD:   // 0xfffd
        if (hmod)
          continue;
        DosLoadModule(0, 0, pszModName, &hmod);
        continue;
      default:
        continue;
    }
    
    break;
  }
  
  initflag = 1;

  DosCloseQueue(hq);
  DosFreeModule(hmod);
  DosCloseMutexSem(hmtx);
}
