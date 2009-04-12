/* $Id: kee.h,v 1.1 2000/04/21 10:58:00 ktk Exp $ */

/* Released to the public domain. All rights perverse */
/*
 * This is not an official IBM header file, and never was.
 * It is published in the hope that some day IBM will document
 * the new KEE32 32 bit driver API properly.
 * There is no warranty that the declarations and symbols are correct
 * and match corresponding official symbols correctly, however the
 * information herein has been collected and analysed carefully. It
 * represents the authors's current knowledge about this API.
 */

#ifndef __KEE_H__
#define __KEE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* The spinlock data type. KEE spinlock functions either put 0 (unused)
 * or 0xff (used) into the variable, and 0xff000000 when the lock is
 * acquired.
 */
typedef ULONG SpinLock_t;
#define  KEESpinLock SpinLock_t

/*ordinal 10 */
APIRET APIENTRY KernAllocSpinLock(SpinLock_t* sl);

/*ordinal 11*/
/* XXX I suppose this routine should receive a SpinLock_t*, but
 * so far, it just returns NO_ERROR, and not even clearing the Spinlock
 * ownership. I think this is a bug in 14.039F_SMP
 */
APIRET APIENTRY KernFreeSpinLock(SpinLock_t* sl);

/*ordinal 12*/
VOID APIENTRY KernAcquireSpinLock(SpinLock_t* sl);

/*ordinal 13*/
VOID APIENTRY KernReleaseSpinLock(SpinLock_t* sl);

typedef struct MutexLock_t
{
    ULONG mtx[2];
} MutexLock_t;
#define KEEMutexLock MutexLock_t


/*ordinal 20*/
APIRET APIENTRY KernAllocMutexLock(MutexLock_t* ml);

/*ordinal 21*/
/* XXX Like ordinal 11, I think this should really do something. Actually,
 * it just returns NO_ERROR. At least, it accesses the argument. But
 * it doesn't do anything. Again, likely an error in 14.039F_SMP
 */
APIRET APIENTRY KernFreeMutexLock(MutexLock_t* ml);

/*ordinal 22*/
VOID APIENTRY KernRequestSharedMutex(MutexLock_t* ml);

/*ordinal 23*/
VOID APIENTRY KernReleaseSharedMutex(MutexLock_t* ml);

/*ordinal 24*/
INT APIENTRY KernTryRequestSharedMutex(MutexLock_t* ml);

/*ordinal 25*/
VOID APIENTRY KernRequestExclusiveMutex(MutexLock_t* ml);

/*ordinal 26*/
VOID APIENTRY KernReleaseExclusiveMutex(MutexLock_t* ml);

/*ordinal 27*/
INT APIENTRY KernTryRequestExclusiveMutex(MutexLock_t* ml);


#define BLOCK_UNINTERRUPTABLE    0x00000001 /* The same as BLOCK_NOSIGNALS. */
#define BLOCK_SPINLOCK           0x00000002
#define BLOCK_EXCLUSIVE_MUTEX    0x00000004
#define BLOCK_SHARED_MUTEX       0x00000008
#define BLOCK_NOACQUIRE          0x00000010

#define KEE_BLOCK_NOSIGNALS             BLOCK_UNINTERRUPTABLE
#define KEE_BLOCK_SPINLOCK              BLOCK_SPINLOCK
#define KEE_BLOCK_EXMUTEXLOCK           BLOCK_EXCLUSIVE_MUTEX
#define KEE_BLOCK_SHMUTEXLOCK           BLOCK_SHARED_MUTEX
#define KEE_BLOCK_NOACQUIRE             BLOCK_NOACQUIRE

/*ordinal 30*/
APIRET APIENTRY KernBlock(ULONG id, ULONG timeout, ULONG flags,
              PVOID ptr, PULONG retdata);

#define WAKEUP_DATA             0x00000008

#define KEE_WAKE_SINGLE         0x00000001
#define KEE_WAKE_PRIOINCR       0x00000004
#define KEE_WAKE_RETDATA        WAKEUP_DATA

/*ordinal 31*/
APIRET APIENTRY KernWakeup(ULONG id, ULONG flags, PULONG ptr, ULONG retdata);

/*ordinal 40*/
VOID APIENTRY KernThunkStackTo16(VOID);

/*ordinal 41*/
VOID APIENTRY KernThunkStackTo32(VOID);

/*ordinal 42*/
VOID APIENTRY KernSerialize16BitDD(VOID);

/*ordinal 43*/
VOID APIENTRY KernUnserialize16BitDD(VOID);

/*ordinal 44*/
VOID APIENTRY KernArmHook(ULONG hook,ULONG data);

/*ordinal 45*/
APIRET APIENTRY KernAllocateContextHook(PVOID pfHandler,ULONG dummy,
                    PULONG pHook);

/*ordinal 50*/
APIRET APIENTRY KernCopyIn(PVOID trgt, PVOID src, ULONG size);

/*ordinal 51*/
APIRET APIENTRY KernCopyOut(PVOID trgt, PVOID src, ULONG size);


/* same bits as with DevHlp_VMAlloc, see explanation there */
#define VMDHA_16MB        0x00000001
#define VMDHA_FIXED       0x00000002
#define VMDHA_SWAP        0x00000004
#define VMDHA_CONTIG      0x00000008
#define VMDHA_PHYS        0x00000010
#define VMDHA_PROCESS     0x00000020
#define VMDHA_SGSCONT     0x00000040
#define VMDHA_GETSEL      0x00000080
#define VMDHA_SELMAP      0x00000080
#define VMDHA_RESERVE     0x00000100
#define VMDHA_SHARED      0x00000400
#define VMDHA_USEHIGHMEM  0x00000800

#define KEE_VMA_16MB       VMDHA_16MB
#define KEE_VMA_FIXED      VMDHA_FIXED
#define KEE_VMA_SWAP       VMDHA_SWAP
#define KEE_VMA_CONTIG     VMDHA_CONTIG
#define KEE_VMA_PHYS       VMDHA_PHYS
#define KEE_VMA_PROCESS    VMDHA_PROCESS
#define KEE_VMA_SGSCONT    VMDHA_SGSCONT
#define KEE_VMA_GETSEL     VMDHA_GETSEL
#define KEE_VMA_RESERVE    VMDHA_RESERVE
#define KEE_VMA_SHARED     VMDHA_SHARED
#define KEE_VMA_USEHIGHMEM VMDHA_USEHIGHMEM

/*ordinal 60*/
APIRET APIENTRY KernVMAlloc(ULONG cb, ULONG flFlags, PVOID* pLinAddr,
                            PVOID* pPhysAddr, PVOID pvSel);

/*ordinal 61*/
APIRET APIENTRY KernVMFree(PVOID pvLinAddr);

/* this is the lockhandle, like with DevHlp_VMLock */
typedef struct {
    UCHAR lock[12];
} KEEVMLock;

/* this is a page list element, like PageList_s in 16 bit Devhlp */
typedef struct {
    ULONG addr;
    ULONG size;
} KEEVMPageList;

/* the same bits as with DevHlp_VMLock */

#define VMDHL_NOBLOCK       0x0001
#define VMDHL_CONTIGUOUS    0x0002
#define VMDHL_16M           0x0004
#define VMDHL_WRITE         0x0008
#define VMDHL_LONG          0x0010
#define VMDHL_VERIFY        0x0020
//#define VMDHL_TRY_CONTIG    0x8000 /* this might perhaps be expanded to 0x80000000? KEE_VML_unknown? */

#define KEE_VML_unknown     0x80000000
#define KEE_VML_NOTBLOCK    VMDHL_NOBLOCK
#define KEE_VML_CONTIG      VMDHL_CONTIGUOUS
#define KEE_VML_16M         VMDHL_16M
#define KEE_VML_WRITEABLE   VMDHL_WRITE
#define KEE_VML_LONGLOCK    VMDHL_LONG
#define KEE_VML_VERIFYONLY  VMDHL_VERIFY

/* ? */
typedef struct KernVMLock_s
{
    char dummy[12]; /* correct size? */
} KernVMLock_t;
#define KEEVMLOCK  KernVMLock_t

/* This is matching with its devhelp equivalent and JFS. */
typedef struct KernPageList_s
{
    ULONG Addr; /* (devhelp called this PhysAddr) */
    ULONG Size;
} KernPageList_t;
#define KEEVMPageList KernPageList_t


/*ordinal 62*/
APIRET APIENTRY KernVMLock(ULONG flags,PVOID linaddr,ULONG size,
               KernVMLock_t* lock, KernPageList_t* pglist,
               PULONG pgsize);

/*ordinal 63*/
APIRET APIENTRY KernVMUnlock(KernVMLock_t* lock);

/*ordinal 64*/
APIRET APIENTRY KernLinToPageList(PVOID linaddr,ULONG size,KernPageList_t* list, PULONG pgcnt);

#define KEE_VMS_UNCOMMIT    0x00000001
#define KEE_VMS_RESIDENT    0x00000002
#define KEE_VMS_SWAPPABLE   0x00000004

/*ordinal 65*/
APIRET APIENTRY KernVMSetMem(ULONG flags, PVOID linaddr, ULONG size);

/*ordinal 66*/
ULONG KernSelToFlat(ULONG addr16);

/*ordinal 70*/
APIRET APIENTRY KernDynamicAPI(PVOID addr, ULONG cnt, ULONG dummy, PUSHORT sel);

/*ordinal 80*/
APIRET APIENTRY KernRASSysTrace(ULONG major,ULONG minor,PVOID buf, ULONG size);

/*ordinal 81*/
APIRET APIENTRY KernPerfSysTrace(ULONG major,ULONG minor,PVOID buf, ULONG size);

/* this is actually a pointer to the SFT entry for the file */
typedef ULONG KEEhfile;

/*ordinal 100*/
APIRET APIENTRY KernLockFile(HFILE hfd,KEEhfile* khfd);

/*ordinal 101*/
APIRET APIENTRY KernUnLockFile(KEEhfile khfd);

/* this is the file size as returned by ordinal 102 */
typedef QWORD KEEfilesize;

/*ordinal 102*/
APIRET APIENTRY KernGetFileSize(KEEhfile khfd,KEEfilesize* sz);

/*ordinal 103*/
APIRET APIENTRY KernTestFileCache(KEEhfile khfd);

/*ordinal 104*/
APIRET APIENTRY KernReadFileAt(KEEhfile khfd,PVOID buf, QWORD off,
                   ULONG nbytes, PULONG nread);

typedef struct KernCacheList_s
{
    ULONG               LinListCount;
    KernPageList_t*     LinearList;
    ULONG               PhysListCount;
    KernPageList_t*     PhysicalList;
} KernCacheList_t;
#define KEECachePages KernCacheList_t

/*ordinal 105*/
APIRET APIENTRY KernReadFileAtCache(KEEhfile khfd,KernCacheList_t** ptr,
                    QWORD off, ULONG nbytes, PULONG nread);

/*ordinal 106*/
APIRET APIENTRY KernReturnFileCache(KEEhfile khfd,KernCacheList_t* ptr);

typedef struct {
    ULONG data[8];
} KEEUnicodeStruct;

/*ordinal 120*/
APIRET APIENTRY KernCreateUconvObject(USHORT codepage, KEEUnicodeStruct* ucs);

/*ordinal 121*/
APIRET APIENTRY KernStrFromUcs(KEEUnicodeStruct* ucs,
                               PCHAR pachTrgt,
                               PUSHORT pucsSrc,
                               ULONG trgtsize,
                               ULONG srcsize);
/*ordinal 122*/
APIRET APIENTRY KernStrToUcs(KEEUnicodeStruct* ucs,
                             PUSHORT pucsTrgt,
                             PCHAR pachSrc,
                             ULONG cucsTrgt,
                             ULONG cchSrc);

#ifdef __cplusplus
}
#endif


#endif
