#ifndef HDLMAN_H
#define HDLMAN_H

#ifdef __cplusplus
  extern "C" {
#endif

#include <os2.h>

VOID hdlInit();
VOID hdlDone();

ULONG hdlNew(ULONG ulTypeId, PVOID pObject);
PVOID hdlGet(ULONG ulTypeId, ULONG ulHandle);
BOOL hdlRemove(ULONG ulTypeId, ULONG ulHandle);

typedef struct _RWMTX {
  HMTX		hMtx;
  HEV		hEV;
  ULONG		ulReadLocks;
} RWMTX, *PRWMTX;


VOID SysRWMutexCreate(PRWMTX psRWMtx);
VOID SysRWMutexDestroy(PRWMTX psRWMtx);
VOID SysRWMutexLockWrite(PRWMTX psRWMtx);
VOID SysRWMutexUnlockWrite(PRWMTX psRWMtx);
VOID SysRWMutexLockRead(PRWMTX psRWMtx);
VOID SysRWMutexUnlockRead(PRWMTX psRWMtx);

#ifdef __cplusplus
  }
#endif

#endif // HDLMAN_H
