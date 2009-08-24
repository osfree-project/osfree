/*    
	LocalMem.c	2.11
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

#include <string.h>

#include "windows.h"

#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "DPMI.h"
#include "BinTypes.h"
#include "KrnTask.h"

typedef	unsigned short TWIN_HLOCAL;

TWIN_HLOCAL WINAPI TWIN_LocalFree(TWIN_HLOCAL hloc);

extern DSCR *LDT;

static BOOL LocalExpand(UINT,UINT);
static BOOL AllocMovableHandleTable(LPBYTE);
static UINT FindNextMovableHandle(LPBYTE);
static BOOL IsMovableHandle(LPBYTE,UINT);

#define PREV_ARENA(p,u)		PUTWORD(lpSegment+p,u)
#define NEXT_ARENA(p,u)		PUTWORD(lpSegment+p+2,u)
#define FREE_SIZE(p,u)		PUTWORD(lpSegment+p+4,u)
#define PREV_FREE(p,u)		PUTWORD(lpSegment+p+6,u)
#define NEXT_FREE(p,u)		PUTWORD(lpSegment+p+8,u)
#define NUM_ITEMS(p,u)		PUTWORD(lpSegment+p+4,u)
#define FIRST_ARENA(p,u)	PUTWORD(lpSegment+p+6,u)
#define FIRST_FREE(p,u)		PUTWORD(lpSegment+p+8,u)
#define LAST_FREE(p,u)		PUTWORD(lpSegment+p+6,u)
#define LAST_ARENA(p,u)		PUTWORD(lpSegment+p+8,u)

#define GET_PREV(p)		GETWORD(lpSegment+p)
#define GET_NEXT(p)		GETWORD(lpSegment+p+2)
#define GET_SIZE(p)		GETWORD(lpSegment+p+4)
#define GET_PREV_FREE(p)	GETWORD(lpSegment+p+6)
#define GET_NEXT_FREE(p)	GETWORD(lpSegment+p+8)
#define GET_NUM_ITEMS(p)	GETWORD(lpSegment+p+4)

#define GET_HANDLE(u,i)		GETWORD(lpSegment+u+2+4*(i))
#define PUT_HANDLE(u,i,h)	PUTWORD(lpSegment+u+2+4*(i),h)
#define GET_LOCK_BYTE(u,i)	*(lpSegment+u+4*i+5)
#define PUT_LOCK_BYTE(u,i,b)	{*(lpSegment+u+4*i+5) = (BYTE)b;}		

#define	INIT_DELTA		0x40

/* Flags for movable handles */
#define LMF_NODISCARD		0x00
#define LMF_DISCARDABLE		0x01
#define LMF_DISCARDED		0x04

#ifdef DEBUG
static BOOL WalkFreeList(LPBYTE);
#endif

BOOL WINAPI
TWIN_LocalInit(UINT uSegment, UINT uStartAddr, UINT uEndAddr)
{
	LPBYTE lpLocalHeapInfo;
	LPBYTE lpSegment;
	UINT uCurr,uPrev,uNext;
	UINT uSize;

	lpSegment = (LPBYTE)(GetPhysicalAddress((WORD)uSegment));
	if (!uStartAddr) {
	    uStartAddr = GetSelectorLimit(uSegment) - uEndAddr;
	    uEndAddr = GetSelectorLimit(uSegment);
	}
	uStartAddr = (uStartAddr + 3) & ~0x3;
	uEndAddr = (uEndAddr + 3) & ~((UINT)3);
	if (uEndAddr > GetSelectorLimit(uSegment))
	    uEndAddr = (GetSelectorLimit(uSegment) + 3) & ~0x3;
	if (uStartAddr < 0x10)
	    uStartAddr = 0x10;
	PUTWORD(lpSegment+6,(WORD)(uStartAddr + 0x10));
	lpLocalHeapInfo = (LPBYTE)(lpSegment + uStartAddr + 0x10);

/* Initialize local heap info structure */

	PUTWORD(lpLocalHeapInfo,0);			/* check */
	PUTWORD(lpLocalHeapInfo+2,0);			/* freeze */
	PUTWORD(lpLocalHeapInfo+4,4);			/* items */
	PUTDWORD(lpLocalHeapInfo+6,uStartAddr);		/* first LA */
	PUTDWORD(lpLocalHeapInfo+0xa,uEndAddr-0xc);	/* last LA */
	*(lpLocalHeapInfo+0xe) = 0;	      /* number of heap compactions */
	*(lpLocalHeapInfo+0xf) = 0;	      /* current discard level */
	PUTWORD(lpLocalHeapInfo+0x10,0L);     /* total bytes discarded so far */
	PUTWORD(lpLocalHeapInfo+0x14,0);      /* pointer to mov.handle table */
	PUTWORD(lpLocalHeapInfo+0x16,0);      /* next table entry */
	PUTWORD(lpLocalHeapInfo+0x18,INIT_DELTA); /* local handle delta */
	if (uEndAddr == ((GetSelectorLimit(uSegment)+3) & ~0x3)) {
					      /* i.e if expanding is possible */
	    PUTWORD(lpLocalHeapInfo+0x1a,1);  /* pointer to expand function */
	}
	else {
	    PUTWORD(lpLocalHeapInfo+0x1a,0);
	}
	PUTWORD(lpLocalHeapInfo+0x1c,0);      /* pointer to statistics table */
	PUTWORD(lpLocalHeapInfo+0x1e,0L);     /* far pointer to LocalNotify */
	PUTWORD(lpLocalHeapInfo+0x22,0);      /* semaphore; for us - number
						 of movable handle tables   */
	PUTWORD(lpLocalHeapInfo+0x24,0x200);  /* min amount to grow DS */
	PUTWORD(lpLocalHeapInfo+0x26,uEndAddr-uStartAddr); /* min heap size */
	memcpy(lpLocalHeapInfo+0x28,"LH",2);  /* signature */

/* Initialize local arenas */

	uCurr = uStartAddr;			/* first block */
	uPrev = uCurr;
	uNext = uCurr + 0xc;
	PREV_ARENA(uCurr,uPrev|1);
	NEXT_ARENA(uCurr,uNext);
	NUM_ITEMS(uCurr,0xa);
	FIRST_ARENA(uCurr,uCurr);
	FIRST_FREE(uCurr,uCurr+0x3c);

	uPrev = uCurr;
	uCurr = uNext;
	uNext = uStartAddr + 0x3c;
	PREV_ARENA(uCurr,uPrev|1);
	NEXT_ARENA(uCurr,uNext);

	uPrev = uCurr;
	uCurr = uNext;
	uNext = uEndAddr - 0xc;
	uSize = uNext - uCurr - 4;
	PREV_ARENA(uCurr,uPrev);
	NEXT_ARENA(uCurr,uNext);
	FREE_SIZE(uCurr,uSize);
	PREV_FREE(uCurr,uStartAddr);
	NEXT_FREE(uCurr,uNext);

	uPrev = uCurr;				/* last block */
	uCurr = uNext;
	PREV_ARENA(uCurr,uPrev|1);
	NEXT_ARENA(uCurr,uNext);
	NUM_ITEMS(uCurr,0xa);
	LAST_FREE(uCurr,uPrev);
	LAST_ARENA(uCurr,uCurr);

	return TRUE;
}

extern ENV *envp_global;
#define MIN_DIFF 0x10

TWIN_HLOCAL WINAPI
TWIN_LocalAlloc(UINT fuAllocFlags, UINT fuAlloc)
{
	LPBYTE lpSegment, lpLocalHeapInfo;
	UINT uFirst,uLast,uFree,uNew,uTemp;
	UINT uSize = 0,uMaxSize = 0;
	UINT uCurr,uPrev,uNext,uPrevFree,uNextFree;
	UINT uHandle=0;
	UINT uNextHandle = 0;
	BYTE bFlag;
	UINT nCount,uExpandFactor;
	BOOL fMovable;

	lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
#ifdef DEBUG
	if (GETWORD(lpSegment+6) == 0)
	    FatalAppExit(0,"LocalMemory corrupted!\n");
#endif
	lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);
	fMovable = fuAllocFlags & LMEM_MOVEABLE;
	if (fuAlloc < 8)
	    fuAlloc = 8;
	if (fMovable) {
	    if (!GETWORD(lpLocalHeapInfo+0x22)) {
		if (!AllocMovableHandleTable(lpSegment))
		    return 0; /* failed to allocate memory for table */
		lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
		lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);
	    }
	    uNextHandle = GETWORD(lpLocalHeapInfo+0x16);
	    if (!uNextHandle) {
		if (!AllocMovableHandleTable(lpSegment)) {
		   return 0;
		}
		lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
		lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);
		uNextHandle = GETWORD(lpLocalHeapInfo+0x16);
	    }
	    uFirst = (UINT)GETDWORD(lpLocalHeapInfo+0x06);
	    uLast = (UINT)GETDWORD(lpLocalHeapInfo+0x0a);
	    uHandle = uNextHandle;
	    uFree = GET_PREV_FREE(uLast);
#ifdef DEBUG
	if (!uFirst || !uLast || !uFree)
	    FatalAppExit(0,"LocalMemory corrupted!\n");
#endif
	    while (((uSize = GET_SIZE(uFree)) < fuAlloc) &&
		(uFree != uFirst)) 
		uFree = GET_PREV_FREE(uFree);
	}
	else {					/* LMEM_FIXED */
	    uFirst = (UINT)GETDWORD(lpLocalHeapInfo+0x06);
	    uLast = (UINT)GETDWORD(lpLocalHeapInfo+0x0a);
	    uFree = GET_NEXT_FREE(uFirst);
#ifdef DEBUG
	if (!uFirst || !uLast || !uFree)
	    FatalAppExit(0,"LocalMemory corrupted!\n");
#endif
	    while (((uSize = GET_SIZE(uFree)) < fuAlloc) &&
			(uFree != uLast)) {
		uFree = GET_NEXT_FREE(uFree);
		uMaxSize = max(uMaxSize,uSize);
	    }
	}
	if ((uFree == uFirst) || (uFree == uLast))
	    uSize = 0;
	if (uSize < fuAlloc) {
	    if (!(GETWORD(lpLocalHeapInfo+0x1a))) /* expanding DS impossible */
		return FALSE;	/* here we should add compression logic */
	    uExpandFactor = (fuAlloc - uMaxSize)/(GETWORD(lpLocalHeapInfo+0x24))
					+ 1; /* min amount to grow DS */
	    if (!LocalExpand(envp_global->reg.ds,uExpandFactor))
		return FALSE;
	    return TWIN_LocalAlloc(fuAllocFlags,fuAlloc);
	}
	uCurr = uFree;
	uPrev = GET_PREV(uCurr);
	uNext = GET_NEXT(uCurr);
	uPrevFree = GET_PREV_FREE(uCurr);
	uNextFree = GET_NEXT_FREE(uCurr);
	if ((UINT)(uSize - fuAlloc) < MIN_DIFF) {
	    PREV_ARENA(uCurr,(uPrev|1));
	    NEXT_FREE(uPrevFree,uNextFree);
	    PREV_FREE(uNextFree,uPrevFree);
	}
	else {	
	    uSize = (fuAlloc + 3) & ~3;
	    if (!fMovable) { 		/* allocate new block in the beginning 
							of the free block */
		uNew = uCurr + uSize + 4;
		PREV_ARENA(uCurr,(uPrev|1));
		PREV_ARENA(uNew,uCurr);
		NEXT_ARENA(uNew,uNext);
		FREE_SIZE(uNew,uNext-uNew-4);
		PREV_FREE(uNew,uPrevFree);
		NEXT_FREE(uNew,uNextFree);
		NEXT_FREE(uPrevFree,uNew);
		PREV_FREE(uNextFree,uNew);
		NEXT_ARENA(uCurr,uNew);
		if (GET_PREV(uNext) & 1) {
		    PREV_ARENA(uNext,uNew|1);
		}
		else {
		    PREV_ARENA(uNext,uNew);
		}
	    }
	    else {	/* carve movable block from the end of the free block */
		uCurr = uNext - uSize - 4;
		PREV_ARENA(uCurr,(uFree|1));
		NEXT_ARENA(uCurr,uNext);
		uTemp = GET_PREV(uNext);
		if (uTemp & 1) {
		    PREV_ARENA(uNext,(uCurr|1));
		}
		else {
		    PREV_ARENA(uNext,uCurr);
		}
		NEXT_ARENA(uFree,uCurr);
		FREE_SIZE(uFree,uCurr-uFree-4);
	    }
	}
/* Increase items count at offset 0x04 in first LA */

	nCount = GET_NUM_ITEMS(uFirst) + 1;
	NUM_ITEMS(uFirst,nCount);
	NUM_ITEMS(uLast,nCount);
	PUTWORD(lpLocalHeapInfo+0x04,(1+GETWORD(lpLocalHeapInfo+0x04)));
		
	if (fuAllocFlags & LMEM_ZEROINIT)
	    memset(lpSegment+uCurr+4,0,fuAlloc);

	if (!fMovable) {
	    uHandle = uCurr + 4;
	}
	else {

/* Fill in entry into movable handle table */

	    PUTWORD(lpSegment+uNextHandle,uCurr+4);
	    if ((fuAllocFlags & LMEM_DISCARDABLE) ||
			!(fuAllocFlags & (LMEM_NODISCARD|LMEM_NOCOMPACT)))
		bFlag = LMF_DISCARDABLE;
	    else
		bFlag = LMF_NODISCARD;
	    *(lpSegment+uNextHandle+2) = bFlag;
	    *(lpSegment+uNextHandle+3) = 0;		/* lock count */
			
/* Find next available movable handle in the table(s) */

	    uNextHandle = FindNextMovableHandle(lpSegment);

/* Put it at offset 0x16 in LocalHeapInfo structure */

	    PUTWORD(lpLocalHeapInfo+0x16,uNextHandle);

	}
	return (TWIN_HLOCAL)uHandle;
}

char NEAR * WINAPI
TWIN_LocalLock(TWIN_HLOCAL hloc)
{
	LPBYTE lpSegment;
	LPBYTE lpHandle;
	BYTE bLockCount;

	if (!hloc)
	    return NULL;
	lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
	lpHandle = lpSegment + hloc;
	if (IsMovableHandle(lpSegment,hloc)) {
	    if (*(lpHandle+2) & LMF_DISCARDED) {
		return (char NEAR *)0;
	    }
	    bLockCount = *(lpHandle+3);
	    if ((bLockCount != 0xff) &&		/* block is free */
			(bLockCount != 0xf0)) { /* block points to next table */
		(*(lpHandle+3))++;		/* increment lock count */
		return (char NEAR *)MAKELONG(GETWORD(lpHandle),0);	
	    }
	    else {
		return (WORD)NULL;	/* attempt to lock free block */
	    }
	}
	else {
	    return (char NEAR *)MAKELONG(hloc,0);
	}
}

BOOL WINAPI
TWIN_LocalUnlock(TWIN_HLOCAL hloc)
{
	LPBYTE lpSegment;
	LPBYTE lpHandle;
	BYTE bLockCount;

	if (!hloc)
		return TRUE;
	lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
	lpHandle = lpSegment + hloc;
	if (IsMovableHandle(lpSegment,hloc)) {
	    bLockCount = *(lpHandle+3);
	    if ((bLockCount != 0xff) &&		/* block is free */
			(bLockCount != 0xf0)) { /* block points to next table */
		if (bLockCount)
		    (*(lpHandle+3))--;	/* decrement lock count */
		return FALSE;
	    }
	    else
		return TRUE;
	}
	else
	    return TRUE;
}

UINT WINAPI
TWIN_LocalCompact(UINT uMinFree)
{
	LPBYTE lpSegment,lpLocalHeapInfo;
	UINT uFirst, uLast, uFree;
	UINT uMaxFreeSize = 0;

	lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
	lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);
	uFirst = (UINT)GETDWORD(lpLocalHeapInfo+0x06);
	uLast = (UINT)GETDWORD(lpLocalHeapInfo+0xa);

	for (uFree = uFirst; uFree != uLast; uFree = GET_NEXT_FREE(uFree))
	    uMaxFreeSize = max(uMaxFreeSize,GET_SIZE(uFree));
	
	if (uMinFree == 0)
	    return uMaxFreeSize;
	else
	    return uMinFree;
}

TWIN_HLOCAL WINAPI
TWIN_LocalReAlloc(TWIN_HLOCAL hloc, UINT fuNewSize, UINT fuFlags)
{
	LPBYTE lpSegment,lpLocalHeapInfo;
	UINT uSize=0, uCurr=0;
	UINT uHandle;
	LPSTR lpBuffer=NULL;
	BOOL fIsFixed = TRUE;
	BOOL fDiscarded = FALSE;

	if (!hloc)
	    return (TWIN_HLOCAL)TRUE;

	lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
	lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);

	if (IsMovableHandle(lpSegment,hloc)) { /* Are we dealing with movable
						  memory block?		*/
	    if (*(lpSegment + hloc + 2) & LMF_DISCARDED)
		fDiscarded = TRUE;
	    else {
		uCurr = GETWORD(lpSegment + hloc) - 4;
		if (fuNewSize == 0) {	/* discard the block */
		    if (*(lpSegment + hloc + 3) != 0) /* block free or locked */
			return (TWIN_HLOCAL)0;
		    if (!(*(lpSegment + hloc + 2) & LMF_DISCARDABLE))
			return (TWIN_HLOCAL)0;
		    *(lpSegment + hloc + 2) = LMF_DISCARDED;
		    TWIN_LocalFree(uCurr + 4);
		    return (TWIN_HLOCAL)hloc;
		}
	    }
	    fIsFixed = FALSE;
	}
	else
	    uCurr = hloc - 4;		/* fixed memory block */
    if (fDiscarded) {
	PUTWORD(lpSegment+hloc+2,0xffff); /* null out movable handle so that */
	PUTWORD(lpSegment+hloc,0); /* it can be used by following TWIN_LocalAlloc */
    }
    else {
	if (!(GET_PREV(uCurr) & 0x1))
	    return (TWIN_HLOCAL)0;		/* invalid handle -- block free */
	uSize = GET_NEXT(uCurr)-uCurr-4;
	if (fuNewSize < 8)
	    fuNewSize = 8;
	lpBuffer = WinMalloc(fuNewSize);
	if (fuNewSize <= uSize)
	    memcpy(lpBuffer,lpSegment+uCurr+4,fuNewSize);
	else
	    memcpy(lpBuffer,lpSegment+uCurr+4,uSize);
	TWIN_LocalFree(hloc);
    }
    if (fIsFixed) {
	uHandle = TWIN_LocalAlloc(LMEM_FIXED,fuNewSize);
	/* We have to reassign lpSegment because it might have been
		 moved by LocalExpand */
	lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
	uCurr = uHandle - 4;
    }
    else {
	PUTWORD(lpLocalHeapInfo+0x16,hloc);
	uHandle = TWIN_LocalAlloc(LMEM_MOVEABLE,fuNewSize);
	/* We have to reassign lpSegment because it might have been
		 moved by LocalExpand */
	lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
	uCurr = GETWORD(lpSegment + uHandle) - 4;
    }
    if (!uHandle)
	return (TWIN_HLOCAL)0;
    if (!fDiscarded) {
	if (fuNewSize <= uSize)
	    memcpy(lpSegment+uCurr+4,lpBuffer,fuNewSize);
	else {
	    memcpy(lpSegment+uCurr+4,lpBuffer,uSize);
	    if (fuFlags & LMEM_ZEROINIT)
		memset(lpSegment+uCurr+4+uSize,0,fuNewSize-uSize);
	}
	WinFree(lpBuffer);
    }

#ifdef DEBUG
    WalkFreeList(lpSegment);
#endif
    return uHandle;
}

UINT WINAPI
TWIN_LocalFlags(TWIN_HLOCAL hloc)
{
	return 1;
}

UINT WINAPI
TWIN_LocalSize(TWIN_HLOCAL hloc)
{
	LPBYTE lpSegment;
	UINT uCurr;
	
	if (!hloc)
	    return FALSE;
	uCurr = hloc - 4;
	lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
	if (IsMovableHandle(lpSegment,hloc)) { /* Are we dealing with movable
						  memory block?		*/
	    uCurr = GETWORD(lpSegment + hloc) - 4;
	}
	else
	    uCurr = hloc - 4;		/* fixed memory block */

	if (!(GET_PREV(uCurr) & 0x1))
	    return 0;        /* invalid handle -- block free */

	return (GET_NEXT(uCurr)-uCurr-4);
}

UINT WINAPI
TWIN_LocalShrink(TWIN_HLOCAL hloc, UINT cbNewSize)
{
    return cbNewSize;
}

TWIN_HLOCAL WINAPI
TWIN_LocalHandle(UINT uMemOffset)
{
    TWIN_HLOCAL hObject = 0;
    LPBYTE lpLocalHeapInfo;
    LPBYTE lpSegment;
    UINT uStartAddress,uEndAddress;
    UINT uCurr,uNext;
    UINT uMovableHandleTable,uLocalHandleDelta;
    int i;

    lpSegment = (LPBYTE)(GetPhysicalAddress(envp_global->reg.ds));
    lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);
    if (*((char *)(lpLocalHeapInfo + 0x28)) != 'L') {	/* LH signature */
	return (TWIN_HLOCAL)0;
    }
    uStartAddress = (UINT)GETDWORD(lpLocalHeapInfo+6);
    uEndAddress = (UINT)GETDWORD(lpLocalHeapInfo + 0xa);
    if (!((uMemOffset >= uStartAddress) && (uMemOffset <= uEndAddress))) {
	return (TWIN_HLOCAL)0;
    }
    for (uCurr=uStartAddress; uCurr<=uEndAddress; uCurr=uNext) {
	uNext = GETWORD(lpSegment + uCurr + 2);
	if ((uMemOffset > uCurr) && (uMemOffset < uNext)) {
	    hObject = uCurr;
	    break;
	}
    }
    if (!hObject) {
	return (TWIN_HLOCAL)0;
    }
    hObject += 4;

/* check if we are dealing with a movable handle */

    uMovableHandleTable = GETWORD(lpLocalHeapInfo+0x0e);
    uLocalHandleDelta = GETWORD(lpLocalHeapInfo+0x18);
    while (uMovableHandleTable) {
	for (i=0;i<(int)uLocalHandleDelta;i++)
	    if (GET_HANDLE(uMovableHandleTable,i) == hObject) {
		hObject = (TWIN_HLOCAL)(uMovableHandleTable+4*i+2);
		return hObject;
	    }
	uMovableHandleTable = GET_HANDLE(uMovableHandleTable,
					uLocalHandleDelta-1);
    }
    return hObject;
}

static BOOL 
LocalExpand(UINT uSegment,UINT uFactor)
{
    LPBYTE lpSegment,lpOldSegment;
    LPBYTE lpLocalHeapInfo;
    UINT uMovableHandleTable,uLocalHandleDelta;
    LPSTR lpTemp;
    DWORD dwSize;
    UINT uLast,uFirst,uPrev,uNext,uPrevFree,uNextFree;
    UINT uMovHandle,uHandle,uFree,uCurr,uTemp;
    UINT uSize, uMovSize;
    UINT uStartAddr,uEndAddr;
    int i;
    BYTE bLockByte;

    lpOldSegment = GetPhysicalAddress(uSegment);
    lpLocalHeapInfo = lpOldSegment + GETWORD(lpOldSegment+6);
    dwSize = GetSelectorLimit(uSegment);
    dwSize += uFactor * GETWORD(lpLocalHeapInfo+0x24);
    if(dwSize > 0x10000) {
	return FALSE;
    }
    if (!TWIN_ReallocSelector(uSegment,dwSize,GHND))
	return FALSE;
    lpSegment = GetPhysicalAddress(uSegment);

    TWIN_LocalExpandStackFix(uSegment, lpSegment, lpOldSegment);

    lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);

/* updating information about local heap */
    uEndAddr = (dwSize + 3) & ~0x3;
    uStartAddr = GETWORD(lpLocalHeapInfo+6);
    PUTWORD(lpLocalHeapInfo+0x26,uEndAddr-uStartAddr);	/* heap size */
    uLast = GETWORD(lpLocalHeapInfo+0xa);	/* former last LA */
    uFirst = (UINT)GETDWORD(lpLocalHeapInfo+0x06);
    PUTDWORD(lpLocalHeapInfo+0xa,uEndAddr-0xc);		/* last LA */
    memcpy(lpSegment+uEndAddr-0xc,lpSegment+uLast,0xc);
    NEXT_ARENA(uEndAddr-0xc,uEndAddr-0xc);
    NEXT_FREE(uEndAddr-0xc,uEndAddr-0xc);
    uPrev = GET_PREV(uLast) & ~1;
    uPrevFree = GET_PREV_FREE(uLast);
    if (uPrev == uPrevFree) { /* just expand the last free block */
	NEXT_FREE(uPrevFree,uEndAddr-0xc);
	FREE_SIZE(uPrevFree,uEndAddr-0xc-uPrevFree-4);
	NEXT_ARENA(uPrev,uEndAddr-0xc);
    }
    else {	/* former last arena becomes last free */
	if (GET_NEXT(uPrev) != uLast)
	    return FALSE;	/* something is terribly wrong */
	uSize = uEndAddr - 0xc - uLast - 4;
	NEXT_ARENA(uLast,uEndAddr-0xc);
	NEXT_FREE(uLast,uEndAddr-0xc);
	FREE_SIZE(uLast,uSize);
	PREV_FREE(uLast,uPrevFree);
	NEXT_FREE(uPrevFree,uLast);
	PREV_FREE(uEndAddr-0xc,uLast);
	PREV_ARENA(uEndAddr-0xc,uLast);
	PREV_ARENA(uLast,uPrev);
    }
    uMovableHandleTable = GETWORD(lpLocalHeapInfo+0x0e);
    uLocalHandleDelta = GETWORD(lpLocalHeapInfo+0x18);
    lpTemp = WinMalloc(dwSize);
    while (uMovableHandleTable) {
	for (i=0; i<(int)uLocalHandleDelta; i++) {
	    uMovHandle = uMovableHandleTable + 2 + 4*i;
	    bLockByte = *(lpSegment+uMovHandle+3);
	    if (bLockByte != 0) 	/* block is not locked */
		continue;
	    uHandle = GETWORD(lpSegment+uMovHandle)-4;
	    uNext = GET_NEXT(uHandle);
	    uMovSize = uNext - uHandle - 4;
	    memcpy(lpTemp, lpSegment+uHandle+4, uMovSize);
	    TWIN_LocalFree(uHandle+4);
	    uFree = GET_PREV_FREE(uEndAddr-0xc);
	    while (((uSize = GET_SIZE(uFree)) < uMovSize) &&
                        ((uPrevFree = GET_PREV_FREE(uFree)) != uFirst))
		uFree = uPrevFree;
	    if ((UINT)(uSize - uMovSize) > MIN_DIFF) {
		uNext = GET_NEXT(uFree);
		uCurr = uNext - uMovSize - 4;
		PREV_ARENA(uCurr,(uFree|1));
		NEXT_ARENA(uCurr,uNext);
		uTemp = GET_PREV(uNext);
		if (uTemp & 1) {
		    PREV_ARENA(uNext,(uCurr|1));
		}
		else {
		    PREV_ARENA(uNext,uCurr);
		}
		NEXT_ARENA(uFree,uCurr);
		FREE_SIZE(uFree,uCurr-uFree-4);
	    }
	    else {		/* we take up the whole block */
		uCurr = uFree;
		uPrev = GET_PREV(uCurr);
		PREV_ARENA(uCurr,(uPrev|1));
		uPrevFree = GET_PREV_FREE(uFree);
		uNextFree = GET_NEXT_FREE(uFree);
		NEXT_FREE(uPrevFree,uNextFree);
		PREV_FREE(uNextFree,uPrevFree);
	    }
	    memcpy(lpSegment+uCurr+4,lpTemp,uMovSize);
	    PUTWORD(lpSegment+uMovableHandleTable+4*i+2,uCurr+4);
	}
	uMovableHandleTable = GET_HANDLE(uMovableHandleTable,
			uLocalHandleDelta-1);
    }
    WinFree(lpTemp);
    return TRUE;
}

static BOOL
AllocMovableHandleTable(LPBYTE lpSegment)
{
    LPBYTE lpOldSegment;
    UINT uSize;
    UINT uLocalHandleDelta,uMovableHandleTable;
    LPBYTE lpLocalHeapInfo;
    UINT uNextHandle,uNumTables;
    UINT uTemp,uTableTemp;
    register int i;

    lpOldSegment = lpSegment;
    lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);
    uLocalHandleDelta = GETWORD(lpLocalHeapInfo+0x18);
    uNumTables = GETWORD(lpLocalHeapInfo+0x22);
    uSize = uLocalHandleDelta*4 + sizeof(WORD);
    if (!(uMovableHandleTable = TWIN_LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT,uSize)))
	return FALSE;

    /* Here we deal with the case when the segment is reallocated */

    lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
    if (lpSegment != lpOldSegment) 
	lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);

    PUTWORD(lpSegment+uMovableHandleTable,uLocalHandleDelta);
    for (i=0;i<(int)uLocalHandleDelta-1;i++)
	PUTWORD(lpSegment+uMovableHandleTable+4*i+4,0xffff);

    /* special case of lock byte -- reserve last entry for a pointer 
	   to the next MovableHandleTable */
    *(lpSegment+uMovableHandleTable+4*uLocalHandleDelta+1) = 0xf0;

    if (uNumTables) {	/* Walk the list of tables, find the previous one,
			   write the pointer to the new table as the last
			   entry of that one				  */
	uTableTemp = GETWORD(lpLocalHeapInfo+0x0e);
	while ((uTemp = GET_HANDLE(uTableTemp,uLocalHandleDelta-1)))
	    uTableTemp = uTemp;
	PUT_HANDLE(uTableTemp,uLocalHandleDelta-1,uMovableHandleTable);
    }
    else		/* we've just allocated the first one */
	PUTWORD(lpLocalHeapInfo+0x0e,uMovableHandleTable);
	
    PUTWORD(lpLocalHeapInfo+0x22,++uNumTables);
    uNextHandle = uMovableHandleTable+2;
    PUTWORD(lpLocalHeapInfo+0x16,uNextHandle);
    return TRUE;
}
    
WORD WINAPI
TWIN_LocalHandleDelta(int nNewDelta)
{
    LPBYTE lpLocalHeapInfo;
    LPBYTE lpSegment;
    UINT uLocalHandleDelta;

    lpSegment = (LPBYTE)(GetPhysicalAddress(envp_global->reg.ds));
    lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);
    uLocalHandleDelta = GETWORD(lpLocalHeapInfo+0x18);
    if (nNewDelta != 0) 
	PUTWORD(lpLocalHeapInfo+0x18,(WORD)nNewDelta);
    return uLocalHandleDelta;
}

TWIN_HLOCAL WINAPI
TWIN_LocalFree(TWIN_HLOCAL hloc)
{
	LPBYTE lpSegment,lpLocalHeapInfo;
	UINT uCurr = 0;
	UINT uPrev,uNext,uFirst,uTemp;
	UINT uPrevFree,uNextFree;
	BOOL fIsNextUsed,fIsPrevUsed;

	if (!hloc)
	    return (TWIN_HLOCAL)TRUE;
	lpSegment = (LPBYTE)GetPhysicalAddress(envp_global->reg.ds);
	lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);
	if (hloc < GETWORD(lpLocalHeapInfo+6))
	    return (TWIN_HLOCAL)TRUE;
	if (IsMovableHandle(lpSegment,hloc)) {
		if (!(*(lpSegment+(UINT)hloc+3))) {
		    PUTWORD(lpSegment+hloc+2,0xffff);
		    if (hloc < GETWORD(lpLocalHeapInfo+0x16))
			PUTWORD(lpLocalHeapInfo+0x16,hloc);
		    uCurr = GETWORD(lpSegment+hloc) - 4;
		}
		else {
		    return hloc;		/* lock count != 0 */
		}
	}
	if (!uCurr)
	    uCurr = (UINT)hloc - 4;
	uPrev = GET_PREV(uCurr);
	uPrev &= ~1;
	uNext = GET_NEXT(uCurr);
	PREV_ARENA(uCurr,uPrev);
	uTemp = GET_PREV(uNext);
	fIsNextUsed = uTemp & 1;
	uTemp = GET_PREV(uPrev);
	fIsPrevUsed = uTemp & 1;
	if (!fIsPrevUsed || !fIsNextUsed) {
	    if (!fIsPrevUsed) {		/* previous block is free */
		NEXT_ARENA(uPrev,uNext);
		FREE_SIZE(uPrev,uNext-uPrev-4);
		if(fIsNextUsed) {
		    PREV_ARENA(uNext,(uPrev|1));
		}
		else  
		    PREV_ARENA(uNext,uPrev);
		uCurr = uPrev;
	    }
	    if (!fIsNextUsed) /* next is free */ {
		uPrev = uCurr;
		uCurr = uNext;
		uNext = GET_NEXT(uCurr);
		NEXT_ARENA(uPrev,uNext);
		uTemp = GET_PREV(uNext);
		fIsNextUsed = uTemp & 1;
		if(fIsNextUsed) {
		    PREV_ARENA(uNext,(uPrev|1));
		}
		else {  
		    PREV_ARENA(uNext,uPrev);
		}
		FREE_SIZE(uPrev,(uNext-uPrev-4));
		if (fIsPrevUsed)
		    uPrevFree = GET_PREV_FREE(uCurr);
		else
		    uPrevFree = GET_PREV_FREE(uPrev);
		uNextFree = GET_NEXT_FREE(uCurr);
		PREV_FREE(uPrev,uPrevFree);
		NEXT_FREE(uPrevFree,uPrev);
		NEXT_FREE(uPrev,uNextFree);
		PREV_FREE(uNextFree,uPrev);
	    }
	}
	else { /* Insert the block into the free list */
	    uPrev |= 1;
	    uFirst = (UINT)GETDWORD(lpLocalHeapInfo+0x06);
	    uTemp = uCurr;
	    while ((uPrev & 1) && ((uPrev &= ~1) != uFirst)) {
		uTemp = uPrev;
		uPrev = GET_PREV(uPrev);
	    }
	    if (uPrev == uFirst)
		uPrevFree = uFirst;
	    else
		uPrevFree = uTemp;
	    uNextFree = GET_NEXT_FREE(uPrevFree);
	    PREV_FREE(uCurr,uPrevFree);
	    NEXT_FREE(uPrevFree,uCurr);
	    PREV_FREE(uNextFree,uCurr);
	    NEXT_FREE(uCurr,uNextFree);
	    FREE_SIZE(uCurr,uNext-uCurr-4);
	}
#ifdef DEBUG
	WalkFreeList(lpSegment);
#endif
	return (TWIN_HLOCAL)0;
}

#ifdef DEBUG
static BOOL
WalkFreeList(LPBYTE lpSegment)
{
    LPBYTE lpLocalHeapInfo;
    UINT uFirst,uLast,uTemp,uCurr;

    lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);
    uFirst = (UINT)GETDWORD(lpLocalHeapInfo+0x06);
    uLast = (UINT)GETDWORD(lpLocalHeapInfo+0xa);
    for (uCurr = uFirst,uTemp = GET_NEXT_FREE(uFirst); 
			uCurr != uLast; uTemp = GET_NEXT_FREE(uCurr))
	if (uTemp == 0) {
	    break;
	}
	else {
	    uCurr = uTemp;
	}

    return(TRUE);
}
#endif

static UINT
FindNextMovableHandle(LPBYTE lpSegment)
{
    LPBYTE lpLocalHeapInfo;
    LPBYTE lpHandle,lpMaxHandle;
    UINT uMovableHandleTable,uLocalHandleDelta;
    UINT uNumTables;
    BYTE bLock;

    lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);
    if (!(uNumTables = GETWORD(lpLocalHeapInfo+0x22)))
	return 0;
    uMovableHandleTable = GETWORD(lpLocalHeapInfo+0x0e);
    uLocalHandleDelta = GETWORD(lpLocalHeapInfo+0x18);

    while (TRUE) {
	lpHandle = lpSegment + uMovableHandleTable + 5;
	lpMaxHandle = lpSegment + uMovableHandleTable + 
			(uLocalHandleDelta-1)*4 + WORD_86;
	while (((bLock = *(lpHandle)) != 0xff) && (lpHandle < lpMaxHandle))
	    lpHandle += 4;
	if (bLock == 0xff)
	    return (lpHandle - 3 - lpSegment);
	uMovableHandleTable = GET_HANDLE(uMovableHandleTable,
				uLocalHandleDelta-1);
	if (!uMovableHandleTable)
	    return 0;
    }
}

static BOOL
IsMovableHandle(LPBYTE lpSegment,UINT uHandle)
{
    LPBYTE lpLocalHeapInfo;
    UINT uMovableHandleTable,uLocalHandleDelta;

    lpLocalHeapInfo = lpSegment + GETWORD(lpSegment+6);
    uMovableHandleTable = GETWORD(lpLocalHeapInfo+0x0e);
    uLocalHandleDelta = GETWORD(lpLocalHeapInfo+0x18);

    while (uMovableHandleTable) {
	if ((uHandle >= uMovableHandleTable) &&
	      (uHandle < (uMovableHandleTable+uLocalHandleDelta*4)))
	    return TRUE;	/* handle lies within the range of this table */
	else	/* check the next table */
	    uMovableHandleTable = GET_HANDLE(uMovableHandleTable,
			uLocalHandleDelta-1);
    }
    return FALSE;
}
