#include <stdlib.h>
#include "hdlman.h"

typedef struct _HDLITEM {
  ULONG         ulTypeId;
  PVOID         pObject;
} HDLITEM, *PHDLITEM;

static PHDLITEM         pasList;
static ULONG            ulCount = 0;
static ULONG            ulFreeCount = 0;
static ULONG            ulFreeHandle = 0xFFFFFFFF;
static RWMTX            rwmtxList;
static ULONG            ulInitTimes = 0;

VOID hdlInit()
{
  if ( (ulInitTimes++) == 0 )
  {
    ulCount = 0;
    ulFreeCount = 0;
    ulFreeHandle = 0xFFFFFFFF;
    SysRWMutexCreate( &rwmtxList );
  }
}

VOID hdlDone()
{
  if ( (--ulInitTimes) == 0 )
  {
    SysRWMutexDestroy( &rwmtxList );
    free( pasList );
  }
}

ULONG hdlNew(ULONG ulTypeId, PVOID pObject)
{
  PHDLITEM              pasListNew;
  PHDLITEM              psNewItem;
  ULONG                 ulHandle;

  SysRWMutexLockWrite( &rwmtxList );

  if ( ulFreeCount != 0 )
  {
    for( psNewItem = &pasList[ulFreeHandle], ulHandle = ulFreeHandle;
         psNewItem->ulTypeId != (ULONG)(-1);
         psNewItem += sizeof(HDLITEM), ulHandle++ )
    {
    }

    ulFreeCount--;
    ulFreeHandle = ulFreeCount != 0 ? ulHandle + 1 : 0xFFFFFFFF;
  }
  else
  {
    if ( ulCount == 0 || (ulCount & 0x07FF) == 0x07FF )
    {
      // realloc list every 2047 items
      pasListNew = realloc( pasList,
                            ( ( ( ulCount + 1 ) & 0xFFFFF800 ) + 0x0800 )
                            * sizeof(HDLITEM) );
      if ( pasListNew == NULL )
      {
        free( pasListNew );
        return (ULONG)(-1);
      }

      pasList = pasListNew;
    }

    ulHandle = ulCount;
    ulCount++;
    psNewItem = &pasList[ulHandle];
  }

  psNewItem->pObject = pObject;
  psNewItem->ulTypeId = ulTypeId;

  SysRWMutexUnlockWrite( &rwmtxList );

  return ulHandle;
}

PVOID hdlGet(ULONG ulTypeId, ULONG ulHandle)
{
  PVOID         pObject;
  PHDLITEM      psItem;

  SysRWMutexLockRead( &rwmtxList );

  if ( ulHandle < ulCount )
  {
    psItem = &pasList[ulHandle];
    pObject = psItem->ulTypeId == ulTypeId ? psItem->pObject : NULL;
  }
  else
    pObject = NULL;

  SysRWMutexUnlockRead( &rwmtxList );

  return pObject;
}

BOOL hdlRemove(ULONG ulTypeId, ULONG ulHandle)
{
  PHDLITEM      psItem;
  BOOL          fRemoved = FALSE;

  SysRWMutexLockWrite( &rwmtxList );

  if ( ulHandle < ulCount )
  {
    psItem = &pasList[ulHandle];
    if ( psItem->ulTypeId == ulTypeId )
    {
      psItem->ulTypeId = (ULONG)(-1);
      ulFreeCount++;
      if ( ulFreeHandle > ulHandle )
        ulFreeHandle = ulHandle;
      fRemoved = TRUE;
    }
  }

  SysRWMutexUnlockRead( &rwmtxList );

  return fRemoved;
}

VOID SysRWMutexCreate(PRWMTX psRWMtx)
{
  SysMutexCreate( &psRWMtx->hMtx );
  SysEventCreate( &psRWMtx->hEV );
  psRWMtx->ulReadLocks = 0;
}

VOID SysRWMutexDestroy(PRWMTX psRWMtx)
{
  SysMutexDestroy( psRWMtx->hMtx );
  SysEventDestroy( psRWMtx->hEV );
}

VOID SysRWMutexLockWrite(PRWMTX psRWMtx)
{
  while( 1 )
  {
    SysMutexLock( psRWMtx->hMtx );
    if ( psRWMtx->ulReadLocks == 0 )
      break;
    SysMutexUnlock( psRWMtx->hMtx );

    SysEventWait( psRWMtx->hEV, SEM_INDEFINITE_WAIT );
    SysEventReset( psRWMtx->hEV );
  }
}

VOID SysRWMutexUnlockWrite(PRWMTX psRWMtx)
{
  SysMutexUnlock( psRWMtx->hMtx );
  SysEventPost( psRWMtx->hEV );
}

VOID SysRWMutexLockRead(PRWMTX psRWMtx)
{
  SysMutexLock( psRWMtx->hMtx );
  psRWMtx->ulReadLocks++;
  SysMutexUnlock( psRWMtx->hMtx );
}

VOID SysRWMutexUnlockRead(PRWMTX psRWMtx)
{
  SysMutexLock( psRWMtx->hMtx );
  if ( psRWMtx->ulReadLocks > 0 )
    psRWMtx->ulReadLocks--;
  SysMutexUnlock( psRWMtx->hMtx );
  SysEventPost( psRWMtx->hEV );
}
