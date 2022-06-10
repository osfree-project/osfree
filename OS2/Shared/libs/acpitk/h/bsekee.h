/*static char *SCCSID = "@(#)bsekee.h   1.00 98/02/03";*/
/***************************************************************************
*
* Module Name: BSEKEE.H
*
* OS/2 Base Include File For KEE
*
* Copyright (c) 1998 IBM Corporation
*
****************************************************************************
*
*
*
****************************************************************************/

#ifndef __BSEKEE__
#define __BSEKEE__

#ifndef OS2DEF_INCLUDED
#define INCL_32
#define INCL_TYPES
#include <os2def.h>
#endif

#ifndef UNIKERN_INCLUDED
#include <unikern.h>
#endif

/*------------------------------------------------------------------------*
 * General Data Types                                                     *
 *------------------------------------------------------------------------*/

//#ifndef __size_t
//#define __size_t
//typedef unsigned long size_t;
//#endif

typedef struct _KernPageList /* PGL */
{
  ULONG Addr;
  ULONG Size;
} KernPageList_t;

/*------------------------------------------------------------------------*
 * Spinlocks                                                              *
 *------------------------------------------------------------------------*/

#pragma pack(1)

typedef struct _SpinLock_t /* SPL */
{
//  char Spinlock[4];
    UCHAR  Owner;       //   CPU number of owner, smp debug only
    UCHAR  Eflag;       //   Byte of eflags, not used
    UCHAR  NestCounter; //   Nest counter, nesting not allowed, not used
    UCHAR  SpinByte;    //   Spin byte, used on SMP only
} SpinLock_t;

VOID    APIENTRY KernAllocSpinLock (SpinLock_t *pSpinLock);
VOID    APIENTRY KernFreeSpinLock (SpinLock_t *pSpinLock);

VOID    APIENTRY KernAcquireSpinLock (SpinLock_t *pSpinLock);
VOID    APIENTRY KernReleaseSpinLock (SpinLock_t *pSpinLock);


/*------------------------------------------------------------------------*
 * Mutexlocks                                                             *
 *------------------------------------------------------------------------*/

typedef struct _MutextLock_t /* MUL */
{
//  char Mutexlock[8];
    USHORT Owner;
    USHORT NestCounter;
    USHORT WaitForRead;
    USHORT WaitForWrite;
} MutexLock_t;

VOID    APIENTRY KernAllocMutexLock (MutexLock_t *pMutexLock);
VOID    APIENTRY KernFreeMutexLock (MutexLock_t *pMutexLock);

VOID    APIENTRY KernRequestExclusiveMutex (MutexLock_t *pMutexLock);
VOID    APIENTRY KernReleaseExclusiveMutex (MutexLock_t *pMutexLock);
BOOL    APIENTRY KernTryRequestExclusiveMutex (MutexLock_t *pMutexLock);

VOID    APIENTRY KernRequestSharedMutex (MutexLock_t *pMutexLock);
VOID    APIENTRY KernReleaseSharedMutex (MutexLock_t *pMutexLock);
BOOL    APIENTRY KernTryRequestSharedMutex (MutexLock_t *pMutexLock);


/*------------------------------------------------------------------------*
 * 16 bit compatability                                                   *
 *------------------------------------------------------------------------*/

VOID    APIENTRY KernThunkStackTo16 (VOID);
VOID    APIENTRY KernThunkStackTo32 (VOID);

VOID    APIENTRY KernSerialize16BitDD (VOID);
VOID    APIENTRY KernUnserialize16BitDD (VOID);

/*------------------------------------------------------------------------*
 * copy buffer to/from user space from/to system space.                   *
 *------------------------------------------------------------------------*/

APIRET  APIENTRY KernCopyIn (VOID  *Destination,
                             VOID  *Source,
                             ULONG  Length);

APIRET  APIENTRY KernCopyOut (VOID   *Destination,
                              VOID   *Source,
                              ULONG  Length);


/*------------------------------------------------------------------------*
 * Memory Management                                                      *
 *------------------------------------------------------------------------*/


typedef struct _KernVMLock_t /* LH */
{
    char Lockhandle[12];
} KernVMLock_t;

APIRET  APIENTRY KernLinToPageList (VOID           *pLinAddr,
                                    ULONG           Size,
                                    KernPageList_t *pPageList,
                                    ULONG          *pPageListCount);

/*
 * LockFlags
 */
#define VMDHL_NOBLOCK     0x01  /* (0000 0001B)
                                 * If set, return if the requested pages are
                                 * not immediately available.
                                 * If clear, the call blocks until the pages
                                 * are available.
                                 */
#define VMDHL_CONTIGUOUS  0x02  /* (0000 0010B)
                                 * If set, the pages to be locked must be
                                 * physically contiguous.  This should be used
                                 * only by device drivers that are about to
                                 * perform contiguous DMA operations on the
                                 * object.
                                 */
#define VMDHL_16M         0x04  /* (0000 0100B)
                                 * If set, the pages to be locked must be below
                                 * the 16MB address line.
                                 * If clear, then the pages may be locked
                                 * anywhere in memory.
                                 */
#define VMDHL_WRITE       0x08  /* (0000 1000B)
                                 * If set, then the PDD plans to write in the
                                 * region of the segment.  This will cause the
                                 * dirty bit to be set in the page table entries
                                 * because the PDD may be doing DMA to the
                                 * pages, which does not set this bit.
                                 */
#define VMDHL_LONG        0x10  /* (0001 0000B)
                                 * If set, then the pages must be locked for
                                 * a long-term duration.  This should be used
                                 * if the PDD intends to keep the pages locked
                                 * for greater than 2 seconds.
                                 */
#define VMDHL_VERIFY      0x20  /* (0010 0000B)
                                 * If set, the region will be "verify only"
                                 * locked.  This means that the page table
                                 * mapping of this region cannot be removed
                                 * or destructively modified.  (no free or
                                 * decommit allowed in the region and cannot
                                 * remove write permission).  The region will
                                 * be physically locked in memory.
                                 * VMDHL_CONTIGUOUS and VMDHL_IGM are invalid
                                 * with this flag.
                                 * VMDHL_LONG and VMDHL_WRITE are valid.
                                 * If VMDHL_WRITE is set memory is verified for
                                 * read/write access.  If it is clear memory is
                                 * verified for read only access.
                                 * All others must be clear.
                                 */
#define VMDHL_TRY_CONTIG  0x8000 /* (1000 0000 0000 0000B)
                                  * If set, don't wait for short term locks,
                                  * just lock non-contiguosly
                                  */

APIRET APIENTRY KernVMLock (ULONG           VMLockFlags,
                            VOID           *pLinAddr,
                            ULONG           cBytes,
                            KernVMLock_t   *pLockHandle,
                            KernPageList_t *pPageList,
                            ULONG          *pPageListCount);

APIRET  APIENTRY KernVMUnlock (KernVMLock_t *pLockHandle);

/*
 * AllocFlags
 */
#define VMDHA_16MB        0x001 /* (0000 0000 00001B)
                                 * If set, the object will be allocated below
                                 * the 16M line
                                 */
#define VMDHA_FIXED       0x002 /* (0000 0000 00010B)
                                 * If set, the object must be in fixed memory
                                 * at all times.
                                 * If clear, then the object may be moved or
                                 * paged out as is necessary.
                                 */
#define VMDHA_SWAP        0x004 /* (0000 0000 0100B)
                                 * If set, swappable memory will be allocated
                                 * for the object.
                                 * If clear, the memory will be movable or
                                 * fixed dependending on the setting of
                                 * VMDHA_FIXED.
                                 * If set, VMDHA_FIXED must be clear.
                                 */
#define VMDHA_CONTIG      0x008 /* (0000 0000 1000B)
                                 * If set, the object needs to be allocated in
                                 * contiguous memory.
                                 * If clear, the the pages may be discontiguous
                                 * in physical memory.
                                 * In order to request contiguous memory, the
                                 * PDD must have also requested fixed memory.
                                 * (VMDHA_FIXED must be set)
                                 */
#define VMDHA_PHYS        0x010 /* (0000 0001 0000B)
                                 * If set, a linear address mapping will be
                                 * obtained for the physical passed in the
                                 * PhysAddr pointer.
                                 */
#define VMDHA_PROCESS     0x020 /* (0000 0010 0000B)
                                 * If set, the linear address returned will be
                                 * in the process address range.
                                 * If clear, the allocation will be done in the
                                 * global address range, that is, accessible
                                 * outside of the current process' context.
                                 */
#define VMDHA_SGSCONT     0x040 /* (0000 0100 0000B)
                                 * If set, the allocated memory can be
                                 * registered under screen group switch control.
                                 * This flag is valid only if mapping is in
                                 * process address range.  (VMDHA_PROCESS must
                                 * be set)
                                 */
#define VMDHA_SELMAP      0x080 /* (0000 1000 0000B)
                                 * If set, provide a selector mapping for the
                                 * linear address range.
                                 */
#define VMDHA_RESERVE     0x100 /* (0001 0000 0000B)
                                 * If set, the memory will only be reserved.
                                 * No committment will be done and any attempt
                                 * to access reserved but not committed memory
                                 * will cause a fatal page fault.
                                 */
#define VMDHIA_USER       0x200 /* (0010 0000 0000B)
                                 * If set, provides user mode access.
                                 * (init time only)
                                 */
#define VMDHA_SHARED      0x400 /* (0100 0000 0000B)
                                 * If set, Allocate shared memory object. This
                                 * flag is only supported for mapping a
                                 * physical address to non-system memory into
                                 * the shared arena. VMDHA_PHYS must be
                                 * specified.
                                 */
#define VMDHA_USEHIGHMEM  0x800 /* (1000 0000 0000B)
                                 * If set, the object will be allocated above
                                 * the 16MB line if possible.  If memory above
                                 * 16MB exists but there is not enough to
                                 * satisfy the request, the memory above 16MB
                                 * will be used first and the remainder taken
                                 * from below 16MB.  If no memory above 16MB
                                 * exists, the allocation will be taken from
                                 * existing memory.
                                 * This bit is valid only during device driver
                                 * initialization.  If this bit is used at any
                                 * other time, KernVMAlloc will return an error.
                                 * All other bits must be clear.
                                 */
#define VMDHA_ALIGN64K 0x1000   /* (0001 0000 0000 0000B)
                                 * If set, the linear address of the object
                                 * will be aligned on 64K boundary.
                                 */

APIRET APIENTRY KernVMAlloc (ULONG   cBytes,
                             ULONG   VMAllocFlags,
                             VOID  **ppLinAddr,
                             VOID  **ppPhysAddr,
                             VOID  **SysReserved);

APIRET  APIENTRY KernVMFree (VOID *pLinAddr);

/*
 * SetMemFlags
 */
#define VMDHS_DECOMMIT    0x01  /* (0000 0001B)
                                 * If set, the address range is decommitted.
                                 * The address range must entirely committed.
                                 */
#define VMDHS_RESIDENT    0x02  /* (0000 0010B)
                                 * If set, the address range is to be made
                                 * resident.  The address range must be
                                 * entirely decommitted, all resident, or all
                                 * swappable.
                                 */
#define VMDHS_SWAP        0x04  /* (0000 0100B)
                                 * If set, the address range is to be made
                                 * swappable.  The address range must be
                                 * entirely decommitted, all resident, or all
                                 * swappable.
                                 */

APIRET APIENTRY KernVMSetMem (ULONG  VMSetMemFlags,
                              VOID  *pLinAddr,
                              ULONG  Size);

ULONG APIENTRY KernSelToFlat (ULONG);

/*------------------------------------------------------------------------*
 * Process Management                                                     *
 *------------------------------------------------------------------------*/

/*
 * BlockFlags
 */

#define BLOCK_UNINTERRUPTABLE 0x01  /* (0000 0001B)
                                     * Block is uninterruptable
                                     */
#define BLOCK_SPINLOCK        0x02  /* (0000 0010B)
                                     * Release spinlock before BLOCK
                                     */
#define BLOCK_EXCLUSIVE_MUTEX 0x04  /* (0000 0100B)
                                     * Release exclusive mutex before BLOCK
                                     */
#define BLOCK_SHARED_MUTEX    0x08  /* (0000 1000B)
                                     * Release shared mutex before BLOCK
                                     */
#define BLOCK_NOACQUIRE       0x10  /* (0001 0000B)
                                     * Do not acquire lock after BLOCK
                                     */

APIRET APIENTRY KernBlock (ULONG  BlockID,      /* Non-zero block id         */
                           ULONG  Timeout,      /* Timeout in ms.            */
                           ULONG  BlockFlags,   /* Flags, see above          */
                           VOID  *pLock,        /* Release lock before block */
                           ULONG *pData);       /* ptr to data from wakeup   */
/*
 * WakeupFlags
 */

#define WAKEUP_ONE   0x01  /* (0000 0001B)
                            * Wake up at most one thread
                            */
                           /* (0000 0010B)
                            * 0x02 is reserved
                            */
#define WAKEUP_BOOST 0x04  /* (0000 0100B)
                            * Boost priority of awakened threads
                            */
#define WAKEUP_DATA  0x08  /* (0000 1000B)
                            * Set wakeup data for awakened threads
                            */

VOID APIENTRY KernWakeup (ULONG  EventID,     /* wake threads blocked on ID */
                          ULONG  WakeupFlags, /* WakeupFlags, see above     */
                          ULONG *pNumThreads, /* Return count of threads awakened */
                          ULONG  Data);       /* Data to pass to awakened threads */

/*------------------------------------------------------------------------*
 * File System                                                            *
 *------------------------------------------------------------------------*/

typedef void * SFTHANDLE;

typedef struct _KernCacheList /* CAL */
{
  ULONG           LinListCount;
  KernPageList_t *LinearList;
  ULONG           PhysListCount;
  KernPageList_t *PhysicalList;
} KernCacheList_t;

APIRET  APIENTRY KernLockFile (HFILE      hFile,
                               SFTHANDLE *hSFT);

APIRET  APIENTRY KernUnLockFile (SFTHANDLE hSFT);

APIRET  APIENTRY KernGetFileSize (SFTHANDLE  hSFT,
                                  QWORD     *FileSize);

BOOL    APIENTRY KernTestFileCache (SFTHANDLE hSFT);

APIRET  APIENTRY KernReadFileAt (SFTHANDLE  hSFT,
                                 VOID      *pBuffer,
                                 QWORD      Offset,
                                 ULONG      cbRead,
                                 ULONG     *cbActual);

APIRET  APIENTRY KernReadFileAtCache (SFTHANDLE         hSFT,
                                      KernCacheList_t **pCacheList,
                                      QWORD             Offset,
                                      ULONG             cbRead,
                                      ULONG            *cbActual);

APIRET  APIENTRY KernReturnFileCache (SFTHANDLE        hSFT,
                                      KernCacheList_t *pCacheList);

/*------------------------------------------------------------------------*
 * Dynamic API                                                            *
 *------------------------------------------------------------------------*/

typedef unsigned short SELECTOR;

/*
 * DynAPIFlags
 */
#define DYNAPI_CALLGATE32  0x0  /* (0000B)
                                 * 0:32 Call Gate
                                 */
#define DYNAPI_ROUTINE32   0x0  /* (0000B)
                                 * 0:32 routine address
                                 */

APIRET  APIENTRY KernDynamicAPI (VOID     *RoutineAddress,
                                 ULONG     ParmCount,
                                 ULONG     DynAPIFlags,
                                 SELECTOR *pCallGateSel);

/*------------------------------------------------------------------------*
 * Trace (RAS and Performance)                                            *
 *------------------------------------------------------------------------*/

APIRET APIENTRY KernRASSysTrace (ULONG  MajorCode,
                                 ULONG  MinorCode,
                                 VOID  *Buffer,
                                 ULONG  BufferLength);

APIRET APIENTRY KernPerfSysTrace (ULONG  MajorCode,
                                  ULONG  MinorCode,
                                  VOID  *Buffer,
                                  ULONG  BufferLength);

/*------------------------------------------------------------------------*
 * UniCode                                                                *
 *------------------------------------------------------------------------*/

APIRET  APIENTRY KernCreateUconvObject (SHORT     codepage,
                                        PUconvObj uhand);

APIRET  APIENTRY KernStrFromUcs (PUconvObj  co,
                                 char      *target,
                                 UniChar   *source,
                                 LONG       len,
                                 LONG       fromlen);

APIRET  APIENTRY KernStrToUcs (PUconvObj  co,
                               UniChar   *target,
                               char      *source,
                               LONG       len,
                               LONG       fromlen);

/*------------------------------------------------------------------------*
 * Context Hooks                                                          *
 *------------------------------------------------------------------------*/

typedef VOID (APIENTRY *PFNCXTHOOK)(ULONG);

APIRET APIENTRY KernAllocateContextHook (PFNCXTHOOK pfnHook,
                                         ULONG      Flags,
                                         ULONG     *HookHandle);

APIRET APIENTRY KernArmHook (ULONG HookHandle,
                             ULONG HookData,
                             ULONG Flags);

// Additions

extern PVOID _KernKEEVersion;
extern PVOID _KernSISData;
extern PVOID _KernLISData;
extern PVOID _KernTKSSBase;
extern PVOID _KernInterruptLevel;

#define	KernKEEVersion		(ULONG)&_KernKEEVersion
#define	KernSISData	 	((struct InfoSegGDT *)&_KernSISData)
#define	KernLISData		((struct InfoSegLDT *)&_KernLISData)
#define	KernInterruptLevel	*(PULONG)&_KernInterruptLevel
#define	KernTKSSBase		*(PULONG)&_KernTKSSBase

void __cdecl KernPrintf (char *, ... );

#pragma pack()

#endif

