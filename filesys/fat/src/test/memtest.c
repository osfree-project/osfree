#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include "os2.h"
#include "portable.h"
#include "msg.h"

#define HEAP_SIZE      (0xFFF0)
#define MAX_SELECTORS  10


static VOID Freeseg(PVOID pv);
static PVOID gdtAlloc(size_t tSize, BOOL fSwappable);

static BOOL   IsBlockFree(PBYTE pMCB);
static ULONG  BlockSize(PBYTE pMCB);
static VOID   SetFree(PBYTE pMCB);
static VOID   SetInUse(PBYTE pMCB);
static VOID   SetBlockSize(PBYTE pMCB, ULONG ulSize);
static void * FindFreeSpace(void * pStart, size_t tSize);

static void * rgpSegment[MAX_SELECTORS] = {0};
static void * rgpMem[1000];

VOID CheckHeap(VOID);

BYTE szLast[256];

ULONG ulMem = 0;

VOID CheckHeap(VOID)
{
USHORT usSel;
BYTE _huge * pHeapStart;
BYTE _huge * pHeapEnd;
BYTE _huge * pWork;
ULONG ulTotal = 0;

   for (usSel = 0; usSel < MAX_SELECTORS; usSel++)
      {
      pHeapStart = rgpSegment[usSel];
      if (!pHeapStart)
         continue;

      pHeapEnd = pHeapStart + HEAP_SIZE;
      pWork = pHeapStart;
      while (pWork < pHeapEnd)
         {
         if (!IsBlockFree(pWork))
            ulTotal += BlockSize(pWork);

         pWork += BlockSize(pWork) + sizeof (ULONG);
         }
      if (pWork != pHeapEnd)
         errmsg("Heap corruption found!");
      }
   if (ulTotal != ulMem)
      {
      fflush(stdout);
      errmsg("Memory lost! %lu %lu %s", ulMem, ulTotal, szLast);
      }


}
/*********************************************************************
* Malloc
*********************************************************************/
void * cdecl Malloc(size_t tSize)
{
USHORT usSel;
void * pRet;


   if (tSize % 2)
      tSize++;


   for (usSel = 0; usSel < MAX_SELECTORS; usSel++)
      {
      if (rgpSegment[usSel])
         {
         pRet = FindFreeSpace(rgpSegment[usSel], tSize);
         if (pRet)
            goto Malloc_exit;
         }
      }

   for (usSel = 0; usSel < MAX_SELECTORS; usSel++)
      {
      if (!rgpSegment[usSel])
         {
         rgpSegment[usSel] = gdtAlloc(HEAP_SIZE, TRUE);
         if (!rgpSegment[usSel])
            {
            errmsg("FAT32: No gdtSelector for heap!");
            return NULL;
            }
         SetBlockSize(rgpSegment[usSel], HEAP_SIZE - sizeof (ULONG));
         SetFree(rgpSegment[usSel]);

         pRet = FindFreeSpace(rgpSegment[usSel], tSize);
         if (pRet)
            goto Malloc_exit;
         }
      }

   pRet = NULL;

Malloc_exit:
   if (pRet)
      ulMem += tSize;

   sprintf(szLast, "malloc %u bytes %p", tSize, pRet);
   printf("%s\n", szLast);

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

   pHeapStart = pStart;
   pHeapEnd = pHeapStart + HEAP_SIZE;

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
         else
            {
            printf("Remaining %lu bytes\n", ulRemaining);
            ulMem += ulRemaining;
            }

         SetInUse(pWork);
         return pWork + sizeof (ULONG);
         }
      pWork += BlockSize(pWork) + sizeof (ULONG);
      }
   return NULL;
}

void cdecl Free(void * pntr)
{
USHORT usSel;
BYTE _huge * pHeapStart;
BYTE _huge * pHeapEnd;
BYTE _huge * pWork;
BYTE _huge * pToFree = pntr;
BYTE _huge * pPrev;
BYTE _huge * pNext;


#if 0
   if (OFFSETOF(pntr) == 0)
      return Freeseg(pntr);
#endif

   for (usSel = 0; usSel < MAX_SELECTORS; usSel++)
      {
      if (SELECTOROF(pntr) == SELECTOROF(rgpSegment[usSel]))
         break;
      }
   if (usSel == MAX_SELECTORS)
      errmsg("FAT32: %lX not found in Free!", pntr);

   ulMem -= *(PULONG)((PBYTE)pntr - sizeof (ULONG));
   sprintf(szLast, "Free %lu bytes", *(PULONG)((PBYTE)pntr - sizeof (ULONG)));
   printf("%s\n", szLast);

   pHeapStart = rgpSegment[usSel];
   pHeapEnd = pHeapStart + HEAP_SIZE;

   pWork = pHeapStart;
   pPrev = NULL;
   while (pWork < pHeapEnd)
      {
      if (pWork + sizeof (ULONG) == pToFree)
         {
         if (pPrev && IsBlockFree(pPrev))
            {
            SetBlockSize(pPrev,
               BlockSize(pPrev) + BlockSize(pWork) + sizeof (ULONG));
            pWork = pPrev;
            }

         pNext = pWork + BlockSize(pWork) + sizeof (ULONG);
         if (pNext < pHeapEnd && IsBlockFree(pNext))
            SetBlockSize(pWork, BlockSize(pWork) + BlockSize(pNext) + sizeof (ULONG));

         SetFree(pWork);
         break;
         }
      else
         pPrev = pWork;

      pWork += BlockSize(pWork) + sizeof (ULONG);
      }
   if (pWork >= pHeapEnd)
      {
      errmsg("FAT32: ERROR: Address not found in Free");
      return;
      }

   /*
      Free selector if no longer needed
   */
   if (usSel > 0 &&
      BlockSize(rgpSegment[usSel]) == (HEAP_SIZE - sizeof (ULONG)) &&
      IsBlockFree(rgpSegment[usSel]))
      {
      Freeseg(rgpSegment[usSel]);
      rgpSegment[usSel] = NULL;
      }

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

PVOID gdtAlloc(size_t tSize, BOOL fSwappable)
{
USHORT rc;
SEL sel;

   rc = DosAllocSeg(tSize, &sel, 0);
   if (rc)
      return NULL;
   printf("Allocating %p\n", MAKEP(sel,0));
   fflush(stdout);
   return MAKEP(sel, 0);
}

VOID Freeseg(PVOID pv)
{
   printf("Freeing %p\n", pv);
   DosFreeSeg(SELECTOROF(pv));
}

int main(void)
{
ULONG ulCount = 0;

   srand(1);
   for (;;)
      {
      USHORT usIndex = (USHORT)(ulCount % 1000);
      size_t tSize = rand();
      if (rgpMem[usIndex])
         Free(rgpMem[usIndex]);
      CheckHeap();

      rgpMem[usIndex] = Malloc(tSize);
      if (rgpMem[usIndex])
         memset(rgpMem[usIndex], 0, tSize);
      CheckHeap();
      ulCount++;
      }
   return 0;

}

