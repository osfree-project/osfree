#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dos.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include "os2.h"
#include "portable.h"
#include "fat32ifs.h"

#define HEAP_SIZE      (0xFFF0)
#define MAX_SELECTORS  100
#define RESERVED_SEGMENT (PVOID)0x00000001


static BOOL   IsBlockFree(PBYTE pMCB);
static ULONG  BlockSize(PBYTE pMCB);
static VOID   SetFree(PBYTE pMCB);
static VOID   SetInUse(PBYTE pMCB);
static VOID   SetBlockSize(PBYTE pMCB, ULONG ulSize);
static void * FindFreeSpace(void * pStart, size_t tSize);
static VOID GetMemAccess(VOID);
static VOID ReleaseMemAccess(VOID);

static void * rgpSegment[MAX_SELECTORS] = {0};
static ULONG ulSemRWMem = 0;

static BOOL fLocked = FALSE;

VOID CheckHeap(VOID)
{
USHORT usSel;
BYTE _huge * pHeapStart;
BYTE _huge * pHeapEnd;
BYTE _huge * pWork;
USHORT rc;

   for (usSel = 0; usSel < MAX_SELECTORS; usSel++)
      {
      pHeapStart = rgpSegment[usSel];
      if (!pHeapStart || pHeapStart == RESERVED_SEGMENT)
         continue;
      rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pHeapStart, HEAP_SIZE);
      if (rc)
         {
         CritMessage("FAT32: Protection VIOLATION in CheckHeap (SYS%d)", rc);
         Message("FAT32: Protection VIOLATION in CheckHeap (SYS%d)", rc);
         return;
         }
      pHeapEnd = pHeapStart + HEAP_SIZE;
      pWork = pHeapStart;
      while (pWork < pHeapEnd)
         pWork += BlockSize(pWork) + sizeof (ULONG);
      if (pWork != pHeapEnd)
         CritMessage("FAT32: Heap corruption found!");
      }
}

/*********************************************************************
* malloc
*********************************************************************/
#ifdef __WATCOM
_WCRTLINK void * malloc(size_t tSize)
#else
void * cdecl malloc(size_t tSize)
#endif
{
USHORT usSel;
void * pRet;

/*   CheckHeap();*/

   //Message("ma000\n");
   GetMemAccess();

   if (tSize % 2)
      tSize++;

if( tSize > 0 )
   {
   //Message("ma001\n");
   for (usSel = 0; usSel < MAX_SELECTORS; usSel++)
      {
      if (rgpSegment[usSel] && rgpSegment[usSel] != RESERVED_SEGMENT)
         {
         //Message("ma002\n");
         pRet = FindFreeSpace(rgpSegment[usSel], tSize);
         if (pRet)
            goto malloc_exit;
         }
      }

   //Message("ma003\n");
   for (usSel = 0; usSel < MAX_SELECTORS; usSel++)
      {
      if (!rgpSegment[usSel])
         {
         //Message("ma004\n");
         rgpSegment[usSel] = RESERVED_SEGMENT;
         rgpSegment[usSel] = gdtAlloc(HEAP_SIZE, TRUE);
         if (!rgpSegment[usSel])
            {
            CritMessage("FAT32: No gdtSelector for heap!");
            pRet = NULL;
            goto malloc_exit;
            }
         //Message("ma005\n");
         SetBlockSize(rgpSegment[usSel], HEAP_SIZE - sizeof (ULONG)); ////
         //Message("ma006\n");
         SetFree(rgpSegment[usSel]);

         //Message("ma007\n");
         pRet = FindFreeSpace(rgpSegment[usSel], tSize);
         if (pRet)
            goto malloc_exit;
         }
      }
   }

   if (f32Parms.fMessageActive & LOG_MEM)
      Message("Malloc failed, calling gdtAlloc");
   //Message("ma008\n");
   pRet = gdtAlloc(tSize ? ( ULONG )tSize : 65536L, TRUE);

malloc_exit:

   if (f32Parms.fMessageActive & LOG_MEM)
      Message("malloc %lu bytes at %lX", tSize ? ( ULONG )tSize : 65536L, pRet);

   //Message("ma009\n");
   ReleaseMemAccess();
   return pRet;
}

/*********************************************************************
* FindFreeSpace
*********************************************************************/
void * FindFreeSpace(void * pStart, size_t tSize)
{
BYTE _huge * pHeapStart;
BYTE _huge * pHeapEnd;
BYTE _huge * pWork;
BYTE _huge * pNext;
USHORT rc;

   pHeapStart = pStart;
   pHeapEnd = pHeapStart + HEAP_SIZE;

   rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pHeapStart, HEAP_SIZE);
   if (rc)
      {
      CritMessage("FAT32: Protection VIOLATION in FindFreeSpace (SYS%d)", rc);
      Message("FAT32: Protection VIOLATION in FindFreeSpace (SYS%d)", rc);
      return NULL;
      }

   pWork = pHeapStart;
   while (pWork < pHeapEnd)
      {
      if (BlockSize(pWork) >= tSize && IsBlockFree(pWork))
         {
         ULONG ulRemaining = BlockSize(pWork) - tSize;
         if (ulRemaining > sizeof (ULONG))
            {
            pNext = pWork + sizeof (ULONG) + tSize;
            SetBlockSize(pNext, BlockSize(pWork) - tSize - sizeof (ULONG));
            SetFree(pNext);
            SetBlockSize(pWork, tSize);
            }

         SetInUse(pWork);
         return pWork + sizeof (ULONG);
         }
      pWork += BlockSize(pWork) + sizeof (ULONG);
      }
   return NULL;
}

#ifdef __WATCOM
_WCRTLINK void free(void * pntr)
#else
void cdecl free(void * pntr)
#endif
{
USHORT usSel;
BYTE _huge * pHeapStart;
BYTE _huge * pHeapEnd;
BYTE _huge * pWork;
BYTE _huge * pToFree = pntr;
BYTE _huge * pPrev;
BYTE _huge * pNext;
USHORT rc;

   if (f32Parms.fMessageActive & LOG_MEM)
      Message("free %lX", pntr);

/*   CheckHeap();*/

   //Message("fr000\n");
   if (OFFSETOF(pntr) == 0)
      {
      freeseg(pntr);
      return;
      }


   //Message("fr001\n");
   GetMemAccess();

   //Message("fr002\n");
   for (usSel = 0; usSel < MAX_SELECTORS; usSel++)
      {
      if (SELECTOROF(pntr) == SELECTOROF(rgpSegment[usSel]))
         break;
      }
   //Message("fr003\n");
   if (usSel == MAX_SELECTORS)
      {
      CritMessage("FAT32: %lX not found in free!", pntr);
      Message("FAT32: %lX not found in free!", pntr);
      ReleaseMemAccess();
      return;
      }

   //Message("fr004\n");
   pHeapStart = rgpSegment[usSel];
   pHeapEnd = pHeapStart + HEAP_SIZE;

   //Message("fr005\n");
   rc = MY_PROBEBUF(PB_OPREAD, (PBYTE)pHeapStart, HEAP_SIZE);
   if (rc)
      {
      CritMessage("FAT32: Protection VIOLATION in free (SYS%d)", rc);
      Message("FAT32: Protection VIOLATION in free (SYS%d)", rc);
      ReleaseMemAccess();
      return;
      }

   pWork = pHeapStart;
   pPrev = NULL;
   //Message("fr006\n");
   while (pWork < pHeapEnd)
      {
      if (pWork + sizeof (ULONG) == pToFree)
         {
         if (pPrev && IsBlockFree(pPrev))
            {
            //Message("fr007\n");
            SetBlockSize(pPrev,
               BlockSize(pPrev) + BlockSize(pWork) + sizeof (ULONG));
            pWork = pPrev;
            }

         //Message("fr008\n");
         pNext = pWork + BlockSize(pWork) + sizeof (ULONG);
         if (pNext < pHeapEnd && IsBlockFree(pNext))
            SetBlockSize(pWork, BlockSize(pWork) + BlockSize(pNext) + sizeof (ULONG));

         //Message("fr009\n");
         SetFree(pWork);
         break;
         }
      else
         pPrev = pWork;

      //Message("fr010\n");
      pWork += BlockSize(pWork) + sizeof (ULONG);
      }
   if (pWork >= pHeapEnd)
      {
      CritMessage("FAT32: ERROR: Address not found in free");
      Message("ERROR: Address not found in free");
      ReleaseMemAccess();
      return;
      }

   /*
      free selector if no longer needed
   */
   //Message("fr011\n");
   if (usSel > 0 &&
      BlockSize(rgpSegment[usSel]) == (HEAP_SIZE - sizeof (ULONG)) &&
      IsBlockFree(rgpSegment[usSel]))
      {
      PBYTE p = rgpSegment[usSel];
      //Message("fr012\n");
      rgpSegment[usSel] = NULL;
      //Message("fr013\n");
      freeseg(p);
      }
   //Message("fr014\n");
   ReleaseMemAccess();
}

BOOL IsBlockFree(PBYTE pMCB)
{
   return (BOOL)(*((PULONG)pMCB) & 1L);
}


ULONG BlockSize(PBYTE pMCB)
{
   return *((PULONG)pMCB) & ~1L;

}

VOID SetFree(PBYTE pMCB)
{
   *((PULONG)pMCB) |= 1L;
}

VOID SetInUse(PBYTE pMCB)
{
   *((PULONG)pMCB) &= ~1L;
}


VOID SetBlockSize(PBYTE pMCB, ULONG ulSize)
{
BOOL fFree = IsBlockFree(pMCB);

   *((PULONG)pMCB) = ulSize;

   if (fFree)
      SetFree(pMCB);
}

VOID GetMemAccess(VOID)
{
   //_disable();
   while (fLocked)
      {
      if (f32Parms.fMessageActive & LOG_WAIT)
         Message("Waiting for a heap access");

      FSH_SEMREQUEST(&ulSemRWMem, TO_INFINITE);
      //DevHelp_ProcBlock((ULONG)GetMemAccess, 1000, 1);
      //_disable();
      }
   fLocked = TRUE;
}

VOID ReleaseMemAccess(VOID)
{
USHORT usCount;

   fLocked = FALSE;
   FSH_SEMCLEAR(&ulSemRWMem);
   //DevHelp_ProcRun((ULONG)GetMemAccess, &usCount);
}
