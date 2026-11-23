#pragma	title("List Box Replacement  --  Version 1.1 -- (HeapMgr.C)")
#pragma	subtitle("   Heap Manager - Interface Definitions")

/* Program name: Listbox.C    Title: A List Box	Replacement		*/
/*									*/
/* OS/2	Developer Magazine, Issue:  Jan	'94, page 66                    */
/* Author:  Mark Benge	   IBM Corp.					*/
/*	    Matt Smith	   Prominare Inc.				*/
/* Description:	 Replacement for OS/2 List Box,	first of a series.	*/
/*									*/
/* Program Requirements:  OS/2 2.x					*/
/*			  IBM C	Set/2					*/
/*			  WATCOM C 386/9.0				*/
/*			  Borland C++ for OS/2				*/
/*			  OS/2 Toolkit					*/

/* Copyright ¸ International Business Machines Corp. 1991-1994		*/
/* Copyright ¸ 1989-1994  Prominare Inc.  All Rights Reserved.		*/

/************************************************************************/
/************************************************************************/
/*		       DISCLAIMER OF WARRANTIES.			*/
/************************************************************************/
/************************************************************************/
/*     The following [enclosed]	code is	source code created by the	*/
/*     authors.	 This source code is  provided to you solely		*/
/*     for the purpose of assisting you	in the development of your	*/
/*     applications.  The code is provided "AS IS", without		*/
/*     warranty	of any kind.  The authors shall	not be liable		*/
/*     for any damages arising out of your use of the source code,	*/
/*     even if they have been advised of the possibility of such	*/
/*     damages.	 It is provided	purely for instructional and		*/
/*     illustrative purposes.						*/
/************************************************************************/
/************************************************************************/

#pragma	info(noext)
#pragma	strings(readonly)

#define	INCL_DOS		   /* Include OS/2 DOS Kernal		*/
#define	INCL_WIN		   /* Include OS/2 PM Windows Interface	*/

#include <os2.h>
#include <stdio.h>

/* #include <memory.h> */
#include <string.h>

#include "entryfld.h"

/* This	module contains	the routines that handle the heap management	*/
/* for the list	box.  All memory allocation requests are handled	*/
/* through these routines.						*/
/*									*/
/* Equivalent command line invocation of each module using the		*/
/* IBM C Set++ Compiler	Version	2.0 is:					*/
/*									*/
/*     Icc -G3e- -O+ -Rn -C -W3	-FoHeapMgr HeapMgr.C			*/

/* Filename:   HeapMgr.C						*/

/*  Version:   1.1							*/
/*  Created:   1993-10-14						*/
/*  Revised:   1994-01-05						*/

/* Routines:   HHEAPMEM	HeapAlloc(ULONG	cbInitial, ULONG cbNewBlks);	*/
/*	       VOID HeapRelease(HHEAPMEM hHeap);			*/
/*	       ULONG HeapSize(HHEAPMEM hHeap);				*/
/*	       VOID HeapStatus(HHEAPMEM	hHeap, PULONG pcBlocks,		*/
/*			       PULONG pulSize, PULONG pulUsed,		*/
/*			       PULONG pulFree, PULONG pulUnused,	*/
/*			       PULONG pulOverhead);			*/
/*	       VOID HeapDisplayStatus(HHEAPMEM hHeap);			*/
/*	       PVOID HeapMalloc(HHEAPMEM hHeap,	ULONG cbSize);		*/
/*	       PVOID HeapCalloc(HHEAPMEM hHeap,	ULONG cItems,		*/
/*				ULONG cbSize);				*/
/*	       PVOID HeapRealloc(HHEAPMEM hHeap, PVOID pv,		*/
/*				 ULONG cbSize);				*/
/*	       VOID HeapFree(HHEAPMEM hHeap, PVOID pv);			*/


/* --------------------------------------------------------------------	*/

typedef	struct _HEAPBLK
   {
   ULONG	      cb;	   /* Block Size			*/
   ULONG	      cbUsed;	   /* Used Block Size			*/
   struct _HEAPBASE  *phbase;	   /* Base Block Pointer		*/
   struct _HEAPBLK   *phblkFree;   /* User Memory Pointer		*/
   } HEAPBLK ;		   /* hblk */

typedef	HEAPBLK	*PHEAPBLK;

typedef	struct _HEAPBASE
   {
   ULONG	     cbInitial;	   /* Initial Heap Size			*/
   ULONG	     cbNewBlks;	   /* New Block	Allocation Size		*/
   PHEAPBLK	     phblkStart;   /* Starting User Memory Pointer	*/
   PHEAPBLK	     phblkLast;	   /* Starting Free User Memory	Pointer	*/
   PHEAPBLK	     phblkFree;	   /* Starting Free List Memory	Pointer	*/
   struct _HEAPBASE *phbaseNext;   /* Next Block Pointer		*/
   struct _HEAPBASE *phbaseLast;   /* Last Block Pointer		*/
   } HEAPBASE ;		   /* hbase */

typedef	HEAPBASE *PHEAPBASE;

#pragma	subtitle("   Heap Manager - Heap Allocation Function")
#pragma	page( )

/* --- HeapAlloc --------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	allocate a heap	for the	user using	*/
/*     heap start size requested along with the	resizing values.	*/
/*     The heap	returns	a handle which is the actual memory address	*/
/*     such that the entire heap can be	released quickly.		*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     ULONG cbInitial;	= Initial Heap Size				*/
/*     ULONG cbNewBlks;	= New Heap Blocks Size				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     HeapAlloc = Heap	Handle						*/
/*									*/
/* --------------------------------------------------------------------	*/

HHEAPMEM HeapAlloc(ULONG cbInitial, ULONG cbNewBlks)

{
PHEAPBASE phbase;		   /* Heap Base	Pointer			*/

		       /* Check	to see if the initial heap size	is the	*/
		       /* default					*/

if ( cbInitial == HALLOC_DEFAULT )
   cbInitial = 32768UL;
		       /* Check	to see if the initial block size is the	*/
		       /* default					*/

if ( cbNewBlks == HALLOC_DEFAULT )
   cbNewBlks = 32768UL;
		       /* Allocate the base memory for the heap	with it	*/
		       /* being	totally	committed			*/

if ( DosAllocMem((PPVOID)(PVOID)&phbase, cbInitial, PAG_READ | PAG_WRITE | PAG_COMMIT) )
   return(0UL);
		       /* Check	to see if the size requested was not an	*/
		       /* even page size in which case more memory has	*/
		       /* been allocated than requested.  In this case,	*/
		       /* the memory routines will utilize the memory	*/
		       /* as though it was normally allocated.		*/
	
if ( cbInitial % 4096UL	)
   phbase->cbInitial  =	((cbInitial / 4096UL) +	1UL) * 4096UL;
else
   phbase->cbInitial  =	cbInitial;

		       /* Check	to see if the new block	size is	not an	*/
		       /* even page size in which case more memory has	*/
		       /* been allocated than requested.  In this case,	*/
		       /* the memory routines will utilize the memory	*/
		       /* as though it was normally allocated.		*/
	
if ( cbNewBlks % 4096UL	)
   phbase->cbNewBlks  =	((cbNewBlks / 4096UL) +	1UL) * 4096UL;
else
   phbase->cbNewBlks  =	cbNewBlks;

		       /* Return the memory address as the handle for	*/
		       /* the heap					*/

return((HHEAPMEM)(phbase->phbaseLast = phbase));
}
#pragma	subtitle("   Heap Manager - Heap Release Function")
#pragma	page( )

/* --- HeapRelease ------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	release	a heap for the user using	*/
/*     the heap	handle that was	returned through the HeapAlloc(	)	*/
/*     function.  The routine walks the	heap blocks releasing each	*/
/*     block.								*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HHEAPMEM	hHeap; = Heap Handle					*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID HeapRelease(HHEAPMEM hHeap)

{
PHEAPBASE phbase = (PHEAPBASE)hHeap;   /* Heap Base Pointer		*/
PHEAPBASE phbaseNext;		   /* Heap Base	Pointer			*/

		       /* Move through the heap	blocks releasing the	*/
		       /* allocated blocks back	to the system		*/
while (	phbase )
   {
   phbaseNext =	phbase->phbaseNext;
   DosFreeMem((PVOID)phbase);
   phbase = phbaseNext;
   }
}

#define	DEBUG_LISTBOX

#if defined(DEBUG_LISTBOX)

#pragma	subtitle("   Heap Manager - Heap Release Function")
#pragma	page( )

/* --- HeapSize	---------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	determine the size of the heap.		*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HHEAPMEM	hHeap; = Heap Handle					*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     HeapSize	= Size of Heap						*/
/*									*/
/* --------------------------------------------------------------------	*/

ULONG HeapSize(HHEAPMEM	hHeap)

{
PHEAPBASE phbase;		   /* Heap Base	Pointer			*/
ULONG	  ulSize = 0UL;		   /* Heap Size	Holder			*/

		       /* Convert the heap handle to the heap base	*/
		       /* address					*/

phbase = (PHEAPBASE)hHeap;

		       /* Move through the heap	blocks counting	the	*/
		       /* allocated blocks size	for the	total memory	*/
		       /* allocated in the heap				*/
while (	phbase )
   {
   ulSize += phbase->cbInitial;
   phbase = phbase->phbaseNext;
   }
		       /* Return the allocated size of the heap		*/
return(ulSize);
}
#pragma	subtitle("   Heap Manager - Heap Status Function")
#pragma	page( )

/* --- HeapStatus -------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	determine the status of	the heap by	*/
/*     gathering various statistics for	the heap.			*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HHEAPMEM	hHeap;	     = Heap Handle				*/
/*     PULONG	pcBlocks;    = Blocks Count Pointer			*/
/*     PULONG	pulSize;     = Total Heap Size Pointer			*/
/*     PULONG	pulUsed;     = Total Used Pointer			*/
/*     PULONG	pulFree;     = Total Free Pointer			*/
/*     PULONG	pulUnused;   = Total Unused Pointer			*/
/*     PULONG	pulOverhead; = Total Overhead Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID HeapStatus(HHEAPMEM hHeap,	PULONG pcBlocks, PULONG	pulSize,
		PULONG pulUsed,	PULONG pulFree,	PULONG pulUnused, PULONG pulOverhead)

{
PHEAPBASE phbase = (PHEAPBASE)hHeap;   /* Heap Base Pointer		*/
PHEAPBLK  phblk;		   /* Heap Base	Pointer			*/

*pcBlocks = *pulSize = *pulUsed	= *pulFree = *pulUnused	= *pulOverhead = 0UL;

		       /* Move through the heap	blocks counting	the	*/
		       /* allocated blocks size	for the	total memory	*/
		       /* allocated in the heap				*/
while (	phbase )
   {
   ++*pcBlocks;
   *pulSize   += phbase->cbInitial;
   *pulUnused += (phbase->cbInitial - sizeof(HEAPBASE));
   *pulOverhead	+= sizeof(HEAPBASE);
   phblk = (PVOID)((PBYTE)phbase + sizeof(HEAPBASE));

   while ( (phblk <= phbase->phblkLast)	&& phblk->cb )
       {
       if ( phblk->cbUsed )
	   {
	   *pulUsed += phblk->cb;
	   *pulOverhead	+= sizeof(HEAPBLK);
	   *pulUnused -= (phblk->cb + sizeof(HEAPBLK));
	   }
       phblk = (PVOID)((PBYTE)phblk + phblk->cb	+ sizeof(HEAPBLK));
       }
   phblk = phbase->phblkFree;

   while ( phblk )
       {
       if ( phblk->cb )
	   {
	   *pulFree += phblk->cb;
	   *pulOverhead	+= sizeof(HEAPBLK);
	   *pulUnused -= (phblk->cb + sizeof(HEAPBLK));
	   }
       phblk = phblk->phblkFree;
       }
   phbase = phbase->phbaseNext;
   }
}
#pragma	subtitle("   Heap Manager - Heap Status Function")
#pragma	page( )

/* --- HeapDisplayStatus ------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	display	the status of the heap by	*/
/*     gathering various statistics for	the heap.			*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HHEAPMEM	hHeap; = Heap Handle					*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID HeapDisplayStatus(HHEAPMEM	hHeap)

{
CHAR  szStrBuf[256];		   /* Display Buffer			*/
ULONG cBlocks;			   /* Blocks Count			*/
ULONG ulFree;			   /* Free Memory Count			*/
ULONG ulOverhead;		   /* Overhead Count			*/
ULONG ulSize;			   /* Total Size Count			*/
ULONG ulUnused;			   /* Unused Count			*/
ULONG ulUsed;			   /* Used Count			*/

if ( hHeap )
   {
   HeapStatus(hHeap, &cBlocks, &ulSize,	&ulUsed, &ulFree, &ulUnused, &ulOverhead);

   sprintf(szStrBuf, "Heap Size: %d\nHeap Used: %d (%d%%)\nHeap Free: %d (%d%%)\nHeap Unused: %d (%d%%)\nHeap Overhead: %d (%d%%)",
		     ulSize, ulUsed, (ulUsed * 100UL) /	ulSize,	ulFree,	(ulFree	* 100UL) / ulSize,
		     ulUnused, (ulUnused * 100UL) / ulSize, ulOverhead,	(ulOverhead * 100UL) / ulSize);
   }
else
   strcpy(szStrBuf, "NULL heap handle.");

WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, szStrBuf,
	      "Heap Debug Status Report", 1UL,
	      MB_OK | MB_ICONEXCLAMATION | MB_MOVEABLE);
}	

#endif

#pragma	subtitle("   Heap Manager - Heap Suballocation Function")
#pragma	page( )

/* --- HeapMalloc -------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	allocate a block of memory from	the	*/
/*     heap using the heap handle that was returned through the		*/
/*     HeapAlloc( ) function.  The routine walks the heap blocks	*/
/*     releasing each looking for either the first free	block		*/
/*     that can	accommodate the	request	or the start of	free memory	*/
/*     which is	first.	If there is no more memory within the heap	*/
/*     block, another block is allocated.  The memory block returned	*/
/*     unlike the traditional malloc( )	will be	initialized to 0	*/
/*     for all of the memory elements.	The placement of each		*/
/*     subblock	is such	that the housekeeping structure	starts on a	*/
/*     4 byte boundary.	 To achieve this, the memory being requested	*/
/*     is padded as required.						*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HHEAPMEM	hHeap;	= Heap Handle					*/
/*     ULONG	cbSize;	= Memory Size Requested				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     HeapMalloc =  0 : Error Return, Out of Memory			*/
/*		  = >0 : Memory	Address					*/
/*									*/
/* --------------------------------------------------------------------	*/

PVOID HeapMalloc(HHEAPMEM hHeap, ULONG cbSize)

{
PHEAPBASE phbase = (PHEAPBASE)hHeap;   /* Heap Base Pointer		*/
PHEAPBASE phbaseNext;		   /* Heap Base	Pointer			*/
PHEAPBASE phbaseStart;		   /* Heap Base	Pointer			*/
PHEAPBLK  phblk;		   /* Heap Base	Pointer			*/
PHEAPBLK  phblkPrev = NULL;	   /* Heap Base	Pointer			*/
PHEAPBLK  phblkNext;		   /* Heap Base	Pointer			*/
ULONG	  cbNewBlks;		   /* New Block	Size			*/
ULONG	  cb = (cbSize / 4L + 1L) * 4L;/* Block	Size			*/

		       /* Walk through the base	addresses looking for	*/
		       /* a subblock of	memory that can	satisfy	the	*/
		       /* request					*/
while (	phbase )
   {
		       /* Get the start	of the free memory subblocks	*/

   if (	(phblk = phbase->phblkFree) != NULL )
       {
       while ( phblk )
	   {
		       /* Have found a subblock	not being used,	see if	*/
		       /* the subblock is large	enough for the request	*/

	   if (	phblk->cb >= cb	)
	       {
	       if ( phblk->cb >	(cb + sizeof(HEAPBLK)) )
		   {
		       /* Form the address of the start	of memory	*/
		       /* subblock by creating the block from the high	*/
		       /* part of the subblock				*/

		   phblkNext = (PHEAPBLK)((PBYTE)phblk + phblk->cb - cb);

		       /* Save the amount of memory being used within	*/
		       /* the subblock					*/

		   phblkNext->cb     = cb;
		   phblkNext->phbase = phbase;

		       /* Adjust the size of the block allocated from	*/
		       /* to be	less the size of the block requested	*/
		       /* and the block	overhead			*/

		   phblk->cb -=	(cb + sizeof(HEAPBLK));

		       /* Set the current pointer to the new subblock	*/

		   phblk = phblkNext;
		   }
	       else
		       /* No subblock division possible, update	the	*/
		       /* first	list pointers to exclude the block	*/

		   if (	phblk == phbase->phblkFree )
		       phbase->phblkFree = phblk->phblkFree;
		   else
		       phblkPrev->phblkFree = phblk->phblkFree;

		       /* Save the requested size in the used portion	*/

	       phblk->cbUsed	= cbSize;
	       phblk->phblkFree	= NULL;

		       /* Clear	the block of memory as is the norm and	*/
		       /* return the address back to the User		*/

	       memset((PVOID)((PBYTE)phblk + sizeof(HEAPBLK)), 0, cbSize);
	       return((PVOID)((PBYTE)phblk + sizeof(HEAPBLK)));
	       }
		       /* Current free list pointer is not adequate for	*/
		       /* the request, check the next one		*/

	   phblkPrev = phblk;
	   phblk = phblkPrev->phblkFree;
	   }
       }
		       /* Check	to see if the last block has been	*/
		       /* defined for the current memory block		*/

   if (	phbase->phblkLast )
       {
		       /* Check	to see if the last block of the	current	*/
		       /* memory block has been	released in which case	*/
		       /* it would not have been added to the free list	*/
		       /* chain	since the last block would then	have	*/
		       /* been undefined				*/

       if ( phbase->phblkLast->cbUsed == 0UL )
	   {
		       /* Last block of	the current memory block	*/
		       /* is a free block, check to see	if the block	*/
		       /* can be sub-divided				*/

	   if (	phbase->phblkLast->cb >= cb )
	       {
	       phblk = phbase->phblkLast;

		       /* See if the block is dividable			*/

	       if ( phbase->phblkLast->cb > (cb	+ sizeof(HEAPBLK)) )
		   {
		       /* Form the address of the start	of memory	*/
		       /* subblock by creating the block from the high	*/
		       /* part of the subblock				*/

		   phblkNext = (PHEAPBLK)((PBYTE)phbase->phblkLast + sizeof(HEAPBLK) + cb);

		       /* Update the next last block pointer to	take	*/
		       /* into consideration the new block being	*/
		       /* allocated					*/

		   phblkNext->phbase	= phbase;
		   phblkNext->cb	= phbase->phblkLast->cb	- (cb +	sizeof(HEAPBLK));
		   phblkNext->cbUsed	= 0UL;
		   phblkNext->phblkFree	= NULL;

		       /* Update the last block	pointer	to point to the	*/
		       /* next block created from the larger block	*/

		   phbase->phblkLast = phblkNext;

		       /* Save the amount of memory being used within	*/
		       /* the subblock					*/

		   phblk->cb	 = cb;
		   }
		       /* Save the requested size in the used portion	*/

	       phblk->cbUsed	= cbSize;
	       phblk->phblkFree	= NULL;

		       /* Clear	the block of memory as is the norm and	*/
		       /* return the address back to the User		*/

	       memset((PVOID)((PBYTE)phblk + sizeof(HEAPBLK)), 0, cbSize);
	       return((PVOID)((PBYTE)phblk + sizeof(HEAPBLK)));
	       }
	   }
       else
		       /* Check	to see if no free subblocks found, in	*/
		       /* which	case a new subblock needs to be	added	*/
		       /* after	the last current subblock		*/

	   if (	(((PBYTE)phbase->phblkLast + (sizeof(HEAPBLK) *	2L) + phbase->phblkLast->cb) + cb) <
		 ((PBYTE)phbase	+ phbase->cbInitial) )
	       {
		       /* Using	the address of the last	subblock,	*/
		       /* calculate the	address	of the new subblock	*/
		       /* such that it is just after the block of	*/
		       /* memory being used by the User			*/

	       phbase->phblkLast = phblk = (PVOID)((PBYTE)phbase->phblkLast + sizeof(HEAPBLK) +	phbase->phblkLast->cb);

		       /* Subblock is adequate for the memory request,	*/
		       /* save the requested size in the used portion	*/

	       phblk->cb	= cb;
	       phblk->cbUsed	= cbSize;

		       /* Save the base	of the memory block to allow	*/
		       /* easy compaction of free memory blocks	when	*/
		       /* memory is released				*/

	       phblk->phbase	= phbase;

		       /* Clear	the block of memory as is the norm and	*/
		       /* return the address back to the User		*/

	       memset((PVOID)((PBYTE)phblk + sizeof(HEAPBLK)), 0, cbSize);
	       return((PVOID)((PBYTE)phblk + sizeof(HEAPBLK)));
	       }
	   }
       else
	   if (	(phbase->cbInitial - sizeof(HEAPBASE) -	sizeof(HEAPBLK)) > cb )
	       {
		       /* No subblocks created yet, start the first	*/
		       /* subblock after the base housekeeping area	*/

	       phbase->phblkLast = phblk = phbase->phblkStart =	(PVOID)((PBYTE)phbase +	sizeof(HEAPBASE));

		       /* Save the requested size in the used portion	*/

	       phblk->cb	= cb;
	       phblk->cbUsed	= cbSize;

		       /* Save the base	of the memory block to allow	*/
		       /* easy compaction of free memory blocks	when	*/
		       /* memory is released				*/

	       phblk->phbase	= phbase;

		       /* Clear	the block of memory as is the norm and	*/
		       /* return the address back to the User		*/

	       memset((PVOID)((PBYTE)phblk + sizeof(HEAPBLK)), 0, cbSize);
	       return((PVOID)((PBYTE)phblk + sizeof(HEAPBLK)));
	       }

   phbase = phbase->phbaseNext;
   }
		       /* Convert the heap handle to the heap base	*/
		       /* address					*/

phbaseStart = (PHEAPBASE)hHeap;

		       /* Point	to the next memory block		*/

phbase = phbaseStart->phbaseLast;

		       /* Last block found, check to see if the	amount	*/
		       /* of memory requested is larger	than the new	*/
		       /* block	size in	which case the full block is	*/
		       /* allocated					*/

if ( cb	> (phbaseStart->cbNewBlks - sizeof(HEAPBASE) - sizeof(HEAPBLK))	)

		       /* Size of memory requested larger than the	*/
		       /* block	size, calculate	the amount of memory	*/
		       /* needed including the housekeeping records	*/

   cbNewBlks = cb + sizeof(HEAPBASE) + sizeof(HEAPBLK);
else
   cbNewBlks = phbaseStart->cbNewBlks;

		       /* Allocate the necessary memory			*/

if ( DosAllocMem((PPVOID)(PVOID)&phbaseNext, cbNewBlks,	PAG_READ | PAG_WRITE | PAG_COMMIT) )

		       /* Error	in allocating the memory, return NULL	*/
		       /* to indicate no memory	was allocated		*/
   return(NULL);
else
		       /* Save the new block address in	the last block	*/
		       /* next pointer					*/

   phbase->phbaseNext =	phbaseStart->phbaseLast	= phbaseNext;

		       /* Check	to see if the new block	size is	not an	*/
		       /* even page size in which case more memory has	*/
		       /* been allocated than requested.  In this case,	*/
		       /* the memory routines will utilize the memory	*/
		       /* as though it was normally allocated.		*/
	
if ( (phbaseNext->cbNewBlks = cbNewBlks) % 4096UL )
   phbaseNext->cbInitial  = ((cbNewBlks	/ 4096UL) + 1) * 4096UL;
else
   phbaseNext->cbInitial  = cbNewBlks;

		       /* Form the address of the start	of memory	*/
		       /* subblock					*/

phbaseNext->phblkLast =	phblk =	phbaseNext->phblkStart = (PVOID)((PBYTE)phbaseNext + sizeof(HEAPBASE));

		       /* Save the amount of memory being used within	*/
		       /* the subblock					*/
phblk->cb     =	cb;
phblk->cbUsed =	cbSize;
		       /* Save the base	of the memory block to allow	*/
		       /* easy compaction of free memory blocks	when	*/
		       /* memory is released				*/

phblk->phbase	 = phbaseNext;

		       /* Return the address of	the User memory	block	*/

return((PVOID)((PBYTE)phblk + sizeof(HEAPBLK)));
}
#pragma	subtitle("   Heap Manager - Heap Suballocation Function")
#pragma	page( )

/* --- HeapCalloc -------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	release	a heap for the user using	*/
/*     the heap	handle that was	returned through the HeapAlloc(	)	*/
/*     function.  The routine uses the HeapMalloc( ) since it is	*/
/*     designed	to clear the memory allocated like a normal		*/
/*     calloc( ) function.						*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HHEAPMEM	hHeap;	= Heap Handle					*/
/*     ULONG	cItems;	= Items	Count					*/
/*     ULONG	cbSize;	= Item Size					*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     HeapCalloc =  0 : Error Return, Out of Memory			*/
/*		  = >0 : Memory	Address					*/
/*									*/
/* --------------------------------------------------------------------	*/

PVOID HeapCalloc(HHEAPMEM hHeap, ULONG cItems, ULONG cbSize)

{
return(HeapMalloc(hHeap, cItems	* cbSize));
}
#pragma	subtitle("   Heap Manager - Heap Re-allocation Function")
#pragma	page( )

/* --- HeapRealloc ------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	reallocate a block of memory for the	*/
/*     user.  If the block is smaller, the same	memory is returned	*/
/*     otherwise a new block is	returned and the old block released.	*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HHEAPMEM	hHeap;	= Heap Handle					*/
/*     PVOID	pv;	= Current Memory Block Address			*/
/*     ULONG	cbSize;	= Required Memory Size				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     HeapRealloc =  0	: Error	Return,	Out of Memory			*/
/*		   = >0	: Memory Address				*/
/*									*/
/* --------------------------------------------------------------------	*/

PVOID HeapRealloc(HHEAPMEM hHeap, PVOID	pv, ULONG cbSize)

{
PVOID	  pvNew;		   /* New Memory Block Address		*/
PHEAPBLK  phblk;		   /* Heap Base	Pointer			*/

		       /* Check	to make	sure that the pointer is not	*/
		       /* NULL which would cause a GP			*/
if ( pv	== NULL	)
   return(NULL);
		       /* Form the address of the subblock housekeeping	*/
		       /* record					*/

phblk =	(PHEAPBLK)((PBYTE)pv - sizeof(HEAPBLK));

		       /* Determine if the size	requested is smaller	*/
		       /* than the current block size in which case the	*/
		       /* the block size should	be used	and returned	*/

if ( phblk->cb >= cbSize )
   {
   phblk->cbUsed = cbSize;
   return(pv);
   }
else
		       /* New size larger than current block size,	*/
		       /* allocate a new block				*/

   if (	(pvNew = HeapMalloc(hHeap, cbSize)) != NULL )
       {
		       /* Transfer the memory from the old block to the	*/
		       /* new block					*/

       memmove(pvNew, pv, phblk->cbUsed);

		       /* Release the memory block for the old block	*/

       HeapFree(hHeap, pv);

		       /* Return the address of	the new	block		*/
       return(pvNew);
       }
   else
		       /* Error	occurred during	the memory allocation,	*/
		       /* return NULL to indicate problem		*/
       return(NULL);
}
#pragma	subtitle("   Heap Manager - Heap Release Function")
#pragma	page( )

/* --- HeapFree	---------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	release	a block	of memory that has	*/
/*     been allocated by the user.  The	routine	performs free block	*/
/*     compaction to allow for proper reallocation of unused blocks.	*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HHEAPMEM	hHeap; = Heap Handle					*/
/*     PVOID	pv;    = Address of User Memory	to Release		*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID HeapFree(HHEAPMEM hHeap, PVOID pv)

{
PHEAPBASE phbase;		   /* Heap Base	Pointer			*/
PHEAPBLK  phblk;		   /* Heap Base	Pointer			*/
PHEAPBLK  phblkNext;		   /* Heap Base	Pointer			*/

		       /* Check	to make	sure that a valid pointer is	*/
		       /* being	released				*/
if ( !hHeap || !pv )
   return;
		       /* Form the address of the subblock housekeeping	*/
		       /* record					*/

phblk =	(PHEAPBLK)((PBYTE)pv - sizeof(HEAPBLK));

if ( phblk->cbUsed )
   {
		       /* Clear	the used component for the subblock	*/
		       /* to indicate that it is freed and make	sure	*/
		       /* that the free	list pointer is	initialized	*/
		       /* as the end of	the chain			*/

   phblk->cbUsed    = 0UL;
   phblk->phblkFree = NULL;
   phbase = phblk->phbase;

		       /* Check	to make	sure that the last block	*/
		       /* pointer is not the one being added to	the	*/
		       /* free list chain				*/

   if (	phbase->phblkLast != phblk )

		       /* Check	to see if the free list	pointer	chain	*/
		       /* has been started				*/

       if ( (phblkNext = phbase->phblkFree) != NULL )
	   {
		       /* Free list pointer chain exists, find the end	*/
		       /* of the chain and add the newly released	*/
		       /* block	to it					*/

	   while ( phblkNext->phblkFree	)
	       phblkNext = phblkNext->phblkFree;

	   phblkNext->phblkFree	= phblk;
	   }
       else
		       /* No free list pointer chain exists, use the	*/
		       /* released block as the	starting point		*/

	   phbase->phblkFree = phblk;
   }
}
