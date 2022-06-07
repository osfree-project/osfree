
/*
 *@@sourcefile procstat.h:
 *      header file for procstat.c (querying process information).
 *      See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\procstat.h"
 */

/*
 *      This file Copyright (C) 1992-2014 Ulrich M”ller,
 *                                        Kai Uwe Rommel.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef PROCSTAT_HEADER_INCLUDED
    #define PROCSTAT_HEADER_INCLUDED

    #pragma pack(1)

    /********************************************************************
     *
     *   DosQProcStatus declarations (16-bit)
     *
     ********************************************************************/

    #define PTR(ptr, ofs)  ((void *) ((char *) (ptr) + (ofs)))

    /* DosQProcStatus() = DOSCALLS.154 */
    USHORT APIENTRY16 DosQProcStatus(PVOID pBuf, USHORT cbBuf);
    /* DosGetPrty = DOSCALLS.9 */
    USHORT APIENTRY16 DosGetPrty(USHORT usScope, PUSHORT pusPriority, USHORT pid);

    /*
     *@@ QPROCSTAT16:
     *      "header" structure returned by DosQProcStat,
     *      containing the offsets to the other data
     */

    typedef struct _QPROCSTAT16
    {
        ULONG  ulGlobal;        // offset to global data section (QGLOBAL16)
        ULONG  ulProcesses;     // offset to process data section (QPROCESS16)
        ULONG  ulSemaphores;    // offset to semaphore data section
        ULONG  ulUnknown1;
        ULONG  ulSharedMem;     // offset to shared mem data section
        ULONG  ulModules;       // offset to DLL data section (QMODULE16)
        ULONG  ulUnknown2;
        ULONG  ulUnknown3;
    } QPROCSTAT16, *PQPROCSTAT16;

    /*
     *@@ QGLOBAL16:
     *      at offset QPROCSTAT.ulGlobal, contains
     *      global system information (no. of threads)
     */

    typedef struct _QGLOBAL16
    {
        ULONG  ulThreads;       // total number of threads;
        ULONG  ulReserved1,
               ulReserved2;
    } QGLOBAL16, *PQGLOBAL16;

    /*
     *@@ QPROCESS16:
     *      DosQProcStat structure for process info
     */

    typedef struct _QPROCESS16
    {
        ULONG  ulType;          // 1 for processes
        ULONG  ulThreadList;    // ofs to array of QTHREAD16 structs
        USHORT usPID;
        USHORT usParentPID;
        ULONG  ulSessionType;
                // according to bsedos.h, the PROG_* types are identical
                // to the SSF_TYPE_* types, so we have:
                // -- PROG_DEFAULT              0
                // -- PROG_FULLSCREEN           1
                // -- PROG_WINDOWABLEVIO        2
                // -- PROG_PM                   3
                // -- PROG_GROUP                5
                // -- PROG_REAL                 4
                // -- PROG_VDM                  4
                // -- PROG_WINDOWEDVDM          7
                // -- PROG_DLL                  6
                // -- PROG_PDD                  8
                // -- PROG_VDD                  9
                // -- PROG_WINDOW_REAL          10
                // -- PROG_WINDOW_PROT          11
                // -- PROG_30_STD               11
                // -- PROG_WINDOW_AUTO          12
                // -- PROG_SEAMLESSVDM          13
                // -- PROG_30_STDSEAMLESSVDM    13
                // -- PROG_SEAMLESSCOMMON       14
                // -- PROG_30_STDSEAMLESSCOMMON 14
                // -- PROG_31_STDSEAMLESSVDM    15
                // -- PROG_31_STDSEAMLESSCOMMON 16
                // -- PROG_31_ENHSEAMLESSVDM    17
                // -- PROG_31_ENHSEAMLESSCOMMON 18
                // -- PROG_31_ENH               19
                // -- PROG_31_STD               20
                // -- PROG_RESERVED             255
        ULONG  ulStatus;        // see status #define's below
        ULONG  ulSID;           // session (screen group) ID
        USHORT usHModule;       // program module handle for process
        USHORT usThreads;       // # of TCBs in use in process
        ULONG  ulReserved1;
        ULONG  ulReserved2;
        USHORT usSemaphores;    // # of 16-bit semaphores
        USHORT usDLLs;          // # of linked DLLs
        USHORT usSharedMems;
        USHORT usReserved3;
        ULONG  ulSemList;       // offset to semaphore list
        ULONG  ulDLLList;       // offset to DLL list
        ULONG  ulSharedMemList; // offset to shared mem list
        ULONG  ulReserved4;
    } QPROCESS16, *PQPROCESS16;

    // process status flags
    #define STAT_EXITLIST 0x01  // processing exit list
    #define STAT_EXIT1    0x02  // exiting thread 1
    #define STAT_EXITALL  0x04  // whole process is exiting
    #define STAT_PARSTAT  0x10  // parent cares about termination
    #define STAT_SYNCH    0x20  // parent did exec-and-wait
    #define STAT_DYING    0x40  // process is dying
    #define STAT_EMBRYO   0x80  // process is in emryonic state

    /*
     *@@ QTHREAD16:
     *      DosQProcStat structure for thread info
     */

    typedef struct _QTHREAD16
    {
        ULONG  ulType;          // 0x100 for thread records
        USHORT usTID;           // thread ID
        USHORT usThreadSlotID;  // ???
        ULONG  ulBlockID;       // sleep id thread is sleeping on
        ULONG  ulPriority;
        ULONG  ulSysTime;
        ULONG  ulUserTime;
        UCHAR  ucStatus;        // TSTAT_* flags
        UCHAR  ucReserved1;
        USHORT usReserved2;
    } QTHREAD16, *PQTHREAD16;

    // thread status flags
    #define TSTAT_VOID          0   // uninitialized
    #define TSTAT_READY         1   // ready to run (waiting for CPU time)
    #define TSTAT_BLOCKED       2   // blocked on a block ID
    #define TSTAT_SUSPENDED     3   // DosSuspendThread
    #define TSTAT_CRITSEC       4   // blocked by another thread in a critical section
    #define TSTAT_RUNNING       5   // currently running
    #define TSTAT_READYBOOST    6   // ready, but apply I/O boost
    #define TSTAT_TSD           7   // thead waiting for thread swappable data (TSD)
    #define TSTAT_DELAYED       8   // delayed TKWakeup (almost ready)
    #define TSTAT_FROZEN        9   // frozen (FF_ICE)
    #define TSTAT_GETSTACK     10   // incoming thread swappable data (TSD)
    #define TSTAT_BADSTACK     11   // TSD failed to swap in

    /*
     *@@ QMODULE16:
     *      DosQProcStat structure for module info
     */

    typedef struct _QMODULE16
    {
        ULONG  nextmodule;
        USHORT modhandle;
        USHORT modtype;
        ULONG  submodules;
        ULONG  segments;
        ULONG  reserved;
        ULONG  namepointer;
        USHORT submodule[1];      // varying, see member submodules */
    } QMODULE16, *PQMODULE16;

    /*
     *@@ QSEMAPHORE16:
     *      DosQProcStat structure for semaphore info (16-bit only, I guess)
     */

    typedef struct _QSEMAPHORE16
    {
        ULONG  nextsem;
        USHORT owner;
        UCHAR  flag;
        UCHAR  refs;
        UCHAR  requests;
        UCHAR  reserved1;
        USHORT reserved2;
        USHORT index;
        USHORT dummy;
        UCHAR  name[1];       /* varying */
    } QSEMAPHORE16, *PQSEMAPHORE16;

    /*
     *@@ QSHAREDMEM16:
     *      DosQProcStat structure for shared memory info
     */

    typedef struct _QSHAREDMEM16
    {
        ULONG  nextseg;
        USHORT handle;            // handle for shared memory
        USHORT selector;          // shared memory selector
        USHORT refs;              // reference count
        UCHAR  name[1];           // varying
    } QSHAREDMEM16, *PQSHAREDMEM16;

    /********************************************************************
     *
     *   DosQuerySysState declarations (32-bit)
     *
     ********************************************************************/

    // #pragma pack(1)

    #define QS32_PROCESS      0x0001
    #define QS32_SEMAPHORE    0x0002
    #define QS32_MTE          0x0004
    #define QS32_FILESYS      0x0008
    #define QS32_SHMEMORY     0x0010
    #define QS32_DISK         0x0020
    #define QS32_HWCONFIG     0x0040
    #define QS32_NAMEDPIPE    0x0080
    #define QS32_THREAD       0x0100
    #define QS32_MODVER       0x0200

/*
    #define QS32_SUPPORTED    (QS32_PROCESS | QS32_SEMAPHORE | QS32_MTE | QS32_FILESYS \
                               | QS32_SHMEMORY | QS32_MODVER)
    QS32_SEMAPHORE is broken V0.9.19 (2002-04-14) [pr]
*/
    #define QS32_SUPPORTED    (QS32_PROCESS | QS32_MTE | QS32_FILESYS \
                               | QS32_SHMEMORY | QS32_MODVER)

    APIRET  APIENTRY DosQuerySysState(ULONG EntityList,
                                      ULONG EntityLevel,
                                      PID pid,
                                      TID tid,
                                      PVOID pDataBuf,
                                      ULONG cbBuf);

    /**********************
     *
     *  global struct
     *
     **********************/

    /*
     *@@ QGLOBAL32:
     *      Pointed to by QTOPLEVEL32.
     */

    typedef struct _QGLOBAL32
    {
        ULONG   ulThreadCount;  // thread count
        // ULONG   ulProcCount;    // process count wrong V1.0.1 (2003-01-10) [umoeller]
        ULONG   ulSem32Count;   // count of 32-bit sems?
        ULONG   ulModuleCount;  // module count
    } QGLOBAL32, *PQGLOBAL32;

    /**********************
     *
     *  thread struct
     *
     **********************/

    /*
     *@@ QTHREAD32:
     *      Pointed to by QPROCESS32.
     */

    typedef struct _QTHREAD32
    {
        ULONG   ulRecType;        // 256 for thread
        USHORT  usTID;          // thread ID, process-specific
        USHORT  usSlotID;       // system-specific slot ID, this identifies the
                                // thread to the kernel
        ULONG   ulSleepID;      // sleep ID the kernel uses for blocking threads
        ULONG   ulPriority;     // priority flags
        ULONG   ulSystime;      // CPU time spent in system code
        ULONG   ulUsertime;     // CPU time spent in user code
        UCHAR   ucState;        // one of the following:
                    // -- TSTAT_READY   1
                    // -- TSTAT_BLOCKED 2
                    // -- TSTAT_RUNNING 5
                    // -- TSTAT_LOADED  9
        UCHAR   _reserved1_;    /* padding to ULONG */
        USHORT  _reserved2_;    /* padding to ULONG */
    } QTHREAD32, *PQTHREAD32;

    /**********************
     *
     *  open files
     *
     **********************/

    // found the following in the "OS/2 Debugging handbook"
    // (the identifiers are not official, but invented by
    // me; V0.9.1 (2000-02-12) [umoeller]):
    // these are the flags for QFDS32.flFlags
    #define FSF_CONSOLEINPUT            0x0001      // bit 0
    #define FSF_CONSOLEOUTPUT           0x0002      // bit 1
    #define FSF_NULLDEVICE              0x0004      // bit 2
    #define FSF_CLOCKDEVICE             0x0008      // bit 3
    // #define FSF_UNUSED1                 0x0010      // bit 4
    #define FSF_RAWMODE                 0x0020      // bit 5
    #define FSF_DEVICEIDNOTDIRTY        0x0040      // bit 6
    #define FSF_LOCALDEVICE             0x0080      // bit 7
    #define FSF_NO_SFT_HANDLE_ALLOCTD   0x0100      // bit 8
    #define FSF_THREAD_BLOCKED_ON_SF    0x0200      // bit 9
    #define FSF_THREAD_BUSY_ON_SF       0x0400      // bit 10
    #define FSF_NAMED_PIPE              0x0800      // bit 11
    #define FSF_SFT_USES_FCB            0x1000      // bit 12
    #define FSF_IS_PIPE                 0x2000      // bit 13;
                // then bit 11 determines whether this pipe is named or unnamed
    // #define FSF_UNUSED2                 0x4000      // bit 14
    #define FSF_REMOTE_FILE             0x8000      // bit 15
                // otherwise local file or device

    /*
     *@@ QFDS32:
     *      open file entry.
     *      Pointed to by QFILEDATA32.
     */

    typedef struct _QFDS32
    {
        USHORT  usSFN;              // "system file number" of the file.
                                    // This is the same as in
                                    // the QPROCESS32.pausFds array,
                                    // so we can identify files opened
                                    // by a process. File handles returned
                                    // by DosOpen ("job file numbers", JFN's)
                                    // are mapped to SFN's for each process
                                    // individually.
        USHORT  usRefCount;
        ULONG   flFlags;            // FSF_* flags above
        ULONG   flAccess;           // fsOpenMode flags of DosOpen:
              /* #define OPEN_ACCESS_READONLY               0x0000
                 #define OPEN_ACCESS_WRITEONLY              0x0001
                 #define OPEN_ACCESS_READWRITE              0x0002
                 #define OPEN_SHARE_DENYREADWRITE           0x0010
                 #define OPEN_SHARE_DENYWRITE               0x0020
                 #define OPEN_SHARE_DENYREAD                0x0030
                 #define OPEN_SHARE_DENYNONE                0x0040
                 #define OPEN_FLAGS_NOINHERIT               0x0080
                 #define OPEN_FLAGS_NO_LOCALITY             0x0000
                 #define OPEN_FLAGS_SEQUENTIAL              0x0100
                 #define OPEN_FLAGS_RANDOM                  0x0200
                 #define OPEN_FLAGS_RANDOMSEQUENTIAL        0x0300
                 #define OPEN_FLAGS_NO_CACHE                0x1000
                 #define OPEN_FLAGS_FAIL_ON_ERROR           0x2000
                 #define OPEN_FLAGS_WRITE_THROUGH           0x4000
                 #define OPEN_FLAGS_DASD                    0x8000
                 #define OPEN_FLAGS_NONSPOOLED          0x00040000
                 #define OPEN_FLAGS_PROTECTED_HANDLE    0x40000000 */

        ULONG   ulFileSize;         // file size in bytes
        USHORT  usHVolume;          // "volume handle"; apparently,
                                    // this identifies some kernel
                                    // structure, it's the same for
                                    // files on the same disk
        USHORT  fsAttribs;          // attributes:
                                    // 0x20: 'A' (archived)
                                    // 0x10: 'D' (directory)
                                    // 0x08: 'L' (?!?)
                                    // 0x04: 'S' (system)
                                    // 0x02: 'H' (hidden)
                                    // 0x01: 'R' (read-only)
        USHORT  us_pad_;
    } QFDS32, *PQFDS32;

    /*
     *@@ QFILEDATA32:
     *      open files linked-list item.
     *
     *      First item is pointed to by QTOPLEVEL32.
     */

    typedef struct _QFILEDATA32
    {
        ULONG           ulRecType;          // 8 for file
        struct _QFILEDATA32 *pNext;         // next record
        ULONG           ulCFiles;           // no. of SFT entries for this MFT entry
        PQFDS32         paFiles;            // first entry here
        char            szFilename[1];
    } QFILEDATA32, *PQFILEDATA32;

    /**********************
     *
     *  process struct
     *
     **********************/

    /*
     *@@ QPROCESS32:
     *      process description structure.
     *
     *      Pointed to by QTOPLEVEL32.
     *
     *      Following this structure is an array
     *      of ulPrivSem32Count 32-bit semaphore
     *      descriptions.
     */

    typedef struct _QPROCESS32
    {
        ULONG       ulRecType;      // 1 for process
        PQTHREAD32  pThreads;       // thread data array,
                                    // apperently with usThreadCount items
        USHORT      usPID;          // process ID
        USHORT      usPPID;         // parent process ID
        ULONG       ulProgType;
                // -- 0: Full screen protected mode.
                // -- 1: Real mode (probably DOS or Windoze).
                // -- 2: VIO windowable protected mode.
                // -- 3: Presentation manager protected mode.
                // -- 4: Detached protected mode.
        ULONG       ulState;    // one of the following:
                // -- STAT_EXITLIST 0x01
                // -- STAT_EXIT1    0x02
                // -- STAT_EXITALL  0x04
                // -- STAT_PARSTAT  0x10
                // -- STAT_SYNCH    0x20
                // -- STAT_DYING    0x40
                // -- STAT_EMBRYO   0x80
        ULONG       ulScreenGroupID; // screen group ID
        USHORT      usHModule;      // module handle of main executable
        USHORT      usThreadCount;  // no. of threads (TCB's in use)
        ULONG       ulPrivSem32Count;  // count of 32-bit semaphores
        PVOID       pvPrivSem32s;   // ptr to 32-bit sems array
        USHORT      usSem16Count;   // count of 16-bit semaphores in pausSem16 array
        USHORT      usModuleCount;  // count of DLLs owned by this process
        USHORT      usShrMemCount;  // count of shared memory handles
        USHORT      usFdsCount;     // count of open files; this is mostly way too large
        PUSHORT     pausSem16;      // ptr to array of 16-bit semaphore handles;
                                    // has usSem16Count items
        PUSHORT     pausModules;    // ptr to array of modules (MTE);
                                    // has usModuleCount items
        PUSHORT     pausShrMems;    // ptr to array of shared mem handles;
                                    // has usShrMemCount items
        PUSHORT     pausFds;        // ptr to array of file handles;
                                    // many of these are pseudo-file handles, but
                                    // will be the same as the QFDS32.sfn field,
                                    // so open files can be identified.
    } QPROCESS32, *PQPROCESS32;

    /**********************
     *
     *  16-bit semaphores
     *
     **********************/

    // SysSemFlag values

    #define QS32_SYSSEM_WAITING 0x01               /* a thread is waiting on the sem */
    #define QS32_SYSSEM_MUXWAITING 0x02            /* a thread is muxwaiting on the sem */
    #define QS32_SYSSEM_OWNER_DIED 0x04            /* the process/thread owning the sem died */
    #define QS32_SYSSEM_EXCLUSIVE 0x08             /* indicates a exclusive system semaphore */
    #define QS32_SYSSEM_NAME_CLEANUP 0x10          /* name table entry needs to be removed */
    #define QS32_SYSSEM_THREAD_OWNER_DIED 0x20     /* the thread owning the sem died */
    #define QS32_SYSSEM_EXITLIST_OWNER 0x40        /* the exitlist thread owns the sem */

    /*
     *@@ QS32SEM16:
     *      16-bit semaphore description.
     *
     *@@added V0.9.10 (2001-04-08) [umoeller]
     */

    typedef struct _QS32SEM16
    {
        struct _QS32SEM16 *pNext;
        // ULONG         NextRec;        /* offset to next record in buffer */
                                      /* System Semaphore Table Structure */
        USHORT        usSysSemOwner;   /* thread owning this semaphore */
        UCHAR         fsSysSemFlags;    /* system semaphore flag bit field */
        UCHAR         usSysSemRefCnt ;  /* number of references to this sys sem */
        UCHAR         usSysSemProcCnt ; /* number of requests for this owner */
        UCHAR         usSysSemPad ;     /* pad byte to round structure up to word */
        USHORT        pad_sh;
        USHORT        SemPtr;         /* RMP SysSemPtr field */
        char          szName[1];      /* start of semaphore name string */
    } QS32SEM16, *PQS32SEM16;

    /*
     *@@ QS32SEM16HEAD:
     *
     *@@added V0.9.10 (2001-04-08) [umoeller]
     */

    typedef struct _QS32SEM16HEAD
    {
        ULONG         SRecType;       /* offset of SysSemDataTable */
        ULONG         SpNextRec;      /* overlays NextRec of 1st QS32SEM16 */
        ULONG         S32SemRec;
        ULONG         S16TblOff;
        // ULONG         pSem16Rec;
        QS32SEM16     Sem16Rec;       // first record, with subsequent following
    } QS32SEM16HEAD, *PQS32SEM16HEAD;

    /**********************
     *
     *  shared memory
     *
     **********************/

    /*
     *@@ QSHRMEM32:
     *      describes a shared memory block.
     *
     *      Pointed to by QTOPLEVEL32.
     */

    typedef struct _QSHRMEM32
    {
        struct _QSHRMEM32 *pNext;
        USHORT      usHandle;           // shared memory handle
        USHORT      usSelector;         // selector
        USHORT      usRefCount;         // reference count
        CHAR        acName[1];          // shared memory name
    } QSHRMEM32, *PQSHRMEM32;

    /**********************
     *
     *  32-bit semaphores
     *
     **********************/

    #define QS32_DC_SEM_SHARED   0x0001   //  Shared Mutex, Event or MUX semaphore
    #define QS32_DCMW_WAIT_ANY   0x0002   //  Wait on any event/mutex to occur
    #define QS32_DCMW_WAIT_ALL   0x0004   //  Wait on all events/mutexs to occur
    #define QS32_DCM_MUTEX_SEM   0x0008   //  Mutex semaphore
    #define QS32_DCE_EVENT_SEM   0x0010   //  Event semaphore
    #define QS32_DCMW_MUX_SEM    0x0020   //  Muxwait semaphore
    // #define QS32_DC_SEM_PM       0x0040   //  PM Shared Event Semphore
    #define QS32_DE_POSTED       0x0040   //  event sem is in the posted state
    #define QS32_DM_OWNER_DIED   0x0080   //  The owning process died
    #define QS32_DMW_MTX_MUX     0x0100   //  MUX contains mutex sems
    #define QS32_DHO_SEM_OPEN    0x0200   //  Device drivers have opened this semaphore
    #define QS32_DE_16BIT_MW     0x0400   //  Part of a 16-bit MuxWait
    #define QS32_DCE_POSTONE     0x0800   //  Post one flag event semaphore
    #define QS32_DCE_AUTORESET   0x1000   //  Auto-reset event semaphore

    /*
     *@@ QS32OPENQ:
     *
     *@@added V0.9.10 (2001-04-08) [umoeller]
     */

    typedef struct _QS32OPENQ {    /* qsopenq */
            PID           pidOpener;      /* process id of opening process */
            USHORT        OpenCt;         /* number of opens for this process */
    } QS32OPENQ, *PQS32OPENQ;

    /*
     *@@ QS32EVENT:
     *
     *@@added V0.9.10 (2001-04-08) [umoeller]
     */

    typedef struct _QS32EVENT {    /* qsevent */
            QS32OPENQ     *pOpenQ;        /* pointer to open q entries */
            UCHAR         *pName;         /* pointer to semaphore name */
            ULONG         *pMuxQ;         /* pointer to the mux queue */
            USHORT        flags;
            USHORT        PostCt;         /* # of posts */
    } QS32EVENT, *PQS32EVENT;

    /*
     *@@ QS32MUTEX:
     *
     *@@added V0.9.10 (2001-04-08) [umoeller]
     */

    typedef struct _QS32MUTEX {    /* qsmutex */
            QS32OPENQ     *pOpenQ;        /* pointer to open q entries */
            UCHAR         *pName;         /* pointer to semaphore name */
            ULONG         *pMuxQ;         /* pointer to the mux queue */
            USHORT        flags;
            USHORT        ReqCt;          /* # of requests */
            USHORT        SlotNum;        /* slot # of owning thread */
            USHORT        pad_sh;
    } QS32MUTEX, *PQS32MUTEX;

    /*
     *@@ QS32MUX:
     *
     *@@added V0.9.10 (2001-04-08) [umoeller]
     */

    typedef struct _QS32MUX {   /* qsmux */
            QS32OPENQ       *pOpenQ;        /* pointer to open q entries */
            UCHAR           *pName;         /* pointer to semaphore name */
            void            *pSemRec;       /* array of semaphore record entries */
            USHORT          flags;
            USHORT          cSemRec;        /* count of semaphore records */
            USHORT          WaitCt;         /* # threads waiting on the mux */
            USHORT          pad_sh;
    } QS32MUX, *PQS32MUX;

    /*
     *@@ QS32SHUN:
     *
     *@@added V0.9.10 (2001-04-08) [umoeller]
     */

    typedef union _QS32SHUN {  /* qsshun */
            QS32EVENT       qsSEvt;         /* shared event sem */
            QS32MUTEX       qsSMtx;         /* shared mutex sem */
            QS32MUX         qsSMux;         /* shared mux sem */
    } QS32SHUN, *PQS32SHUN;

    /*
     *@@ QS32SEM32:
     *
     *@@added V0.9.10 (2001-04-08) [umoeller]
     */

    typedef struct _QS32SEM32 {   /* qsS32rec */
            void        *pNext;      /* pointer to next record in buffer */
            ULONG       fl;         // semaphore flags; THIS FIELD IS MISSING
                                    // IN THE ORIGINAL TOOLKIT DEFINITIONS...
                                    // only with this field we can determine
                                    // if this is a mutex, event, or muxwait
            PSZ         pszName;        // or NULL if unnamed
            ULONG       pvDeviceDriver; // ?!? points into kernel memory
            USHORT      usPostCount;
            USHORT      us_;
            ULONG       ulBlockID;

            // can't make sense of the following fields... these
            // seem to be variable in size
            ULONG       ulHandle;
            /* USHORT      usAlways1;
            ULONG       ulElse;
            ULONG       ulElse2; */
    } QS32SEM32, *PQS32SEM32;           // qsS32rec_t;

    /**********************
     *
     *  modules
     *
     **********************/

    /*
     *@@ QS32OBJ:
     *      describes an object in a module.
     *      Pointed to by QSMODULE32, but only
     *      if QS32_MTE was set on query.
     *
     *@@added V0.9.10 (2001-04-08) [umoeller]
     */

    typedef struct _QS32OBJ {
            ULONG   oaddr;  /* object address */
            ULONG   osize;  /* object size */
            ULONG   oflags; /* object flags */
    } QS32OBJ, *PQS32OBJ;

    /*
     *@@ QMODULE32:
     *      describes an executable module.
     *
     *      Pointed to by QTOPLEVEL32.
     */

    typedef struct _QMODULE32
    {
        struct _QMODULE32 *pNext;       // next module
        USHORT      usHModule;          // module handle (HMTE)
        USHORT      fFlat;              // TRUE for 32-bit modules
        ULONG       ulRefCount;         // no. of imports
        ULONG       cObjects;           // no. of objects in module
        PQS32OBJ    paObjects;          // ptr to objects list, if QS32_MTE was queried
        PCHAR       pcName;             // module name (fully qualified)
        USHORT      ausModRef[1];       // array of module "references";
                                        // this has usRefCount items
                                        // and holds other modules (imports)
    } QMODULE32, *PQMODULE32;

    /**********************
     *
     *  top-level struct
     *
     **********************/

    /*
     *@@ QTOPLEVEL32:
     *      head of the buffer returned by
     *      DosQuerySysState.
     */

    typedef struct _QTOPLEVEL32
    {
        PQGLOBAL32      pGlobalData;
        PQPROCESS32     pProcessData;
        PQS32SEM16HEAD  pSem16Data;
        PQS32SEM32      pSem32Data;     // not always present!
        PQSHRMEM32      pShrMemData;
        PQMODULE32      pModuleData;
        PVOID           _reserved2_;
        PQFILEDATA32    pFileData;      // only present in FP19 or later or W4
    } QTOPLEVEL32, *PQTOPLEVEL32;

    /********************************************************************
     *
     *   New procstat.c declarations
     *
     ********************************************************************/

    /*
     *@@ PRCPROCESS:
     *      additional, more lucid structure
     *      filled by prc16QueryProcessInfo.
     */

    typedef struct _PRCPROCESS
    {
        CHAR   szModuleName[CCHMAXPATH];    // module name
        USHORT usPID,                       // process ID
               usParentPID,                 // parent process ID
               usThreads;                   // thread count
        ULONG  ulSID;                       // session ID
        ULONG  ulSessionType;
        ULONG  ulStatus;
        ULONG  ulCPU;                       // CPU usage (sum of thread data)
    } PRCPROCESS, *PPRCPROCESS;

    /*
     *@@ PRCTHREAD:
     *      additional, more lucid structure
     *      filled by prc16QueryThreadInfo.
     */

    typedef struct _PRCTHREAD
    {
        USHORT usTID;           // thread ID
        USHORT usThreadSlotID;  // kernel thread slot ID
        ULONG  ulBlockID;       // sleep id thread is sleeping on
        ULONG  ulPriority;
        ULONG  ulSysTime;
        ULONG  ulUserTime;
        UCHAR  ucStatus;        // see status #define's below
    } PRCTHREAD, *PPRCTHREAD;

    #pragma pack()

    /********************************************************************
     *
     *   DosQProcStat (16-bit) interface
     *
     ********************************************************************/

    APIRET prc16GetInfo(PQPROCSTAT16 *ppps);

    APIRET prc16FreeInfo(PQPROCSTAT16 pInfo);

    PQPROCESS16 prc16FindProcessFromName(PQPROCSTAT16 pInfo,
                                         const char *pcszName);

    PQPROCESS16 prc16FindProcessFromPID(PQPROCSTAT16 pInfo,
                                        ULONG ulPID);

    /********************************************************************
     *
     *   DosQProcStat (16-bit) helpers
     *
     ********************************************************************/

    BOOL prc16QueryProcessInfo(PQPROCSTAT16 pps, USHORT usPID, PPRCPROCESS pprcp);

    ULONG prc16ForEachProcess(PFNWP pfnwpCallback, HWND hwnd, ULONG ulMsg, MPARAM mp1);

    ULONG prc16QueryThreadCount(PQPROCSTAT16 pps, USHORT usPID);

    BOOL prc16QueryThreadInfo(PQPROCSTAT16 pps, USHORT usPID, USHORT usTID, PPRCTHREAD pprct);

    ULONG prc16QueryThreadPriority(PQPROCSTAT16 pps, USHORT usPID, USHORT usTID);

    /********************************************************************
     *
     *   DosQuerySysState (32-bit) interface
     *
     ********************************************************************/

    PQTOPLEVEL32 prc32GetInfo2(ULONG fl,
                               APIRET *parc);

    PQTOPLEVEL32 prc32GetInfo(APIRET *parc);

    VOID prc32FreeInfo(PQTOPLEVEL32 pInfo);

    PQPROCESS32 prc32FindProcessFromName(PQTOPLEVEL32 pInfo,
                                         const char *pcszName);

    PQPROCESS32 prc32FindProcessFromPID(PQTOPLEVEL32 pInfo,
                                        ULONG pid);

    PQS32SEM16 prc32FindSem16(PQTOPLEVEL32 pInfo,
                              USHORT usSemID);

    PQS32SEM32 prc32FindSem32(PQTOPLEVEL32 pInfo,
                              USHORT usSemID);

    PQSHRMEM32 prc32FindShrMem(PQTOPLEVEL32 pInfo,
                               USHORT usShrMemID);

    PQMODULE32 prc32FindModule(PQTOPLEVEL32 pInfo,
                               USHORT usHModule);

    PQFILEDATA32 prc32FindFileData(PQTOPLEVEL32 pInfo,
                                   USHORT usFileID);

    void prc32KillProcessTree(ULONG pid);

    void prc32KillProcessTree2(PQPROCESS32 pProcThis, ULONG pid);

#endif

#if __cplusplus
}
#endif

