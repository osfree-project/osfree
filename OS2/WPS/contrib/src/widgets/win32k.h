/* $Id$
 *
 * Top level make file for the Win32k library.
 * Contains library and 32-bit IOCtl definition.
 *
 * Copyright (c) 2000 knut st. osmundsen (knut.stange.osmundsen@pmsc.no)
 *
 * Project Odin Software License can be found in LICENSE.TXT
 *
 */
/* NOINC */
#ifndef _WIN32K_H_
#define _WIN32K_H_
/* INC */

/*******************************************************************************
*   Defined Constants And Macros                                               *
*******************************************************************************/
/*
 * IOCtls categories.
 */
#define IOCTL_W32K_K32          0xC1
#define IOCTL_W32K_ELF          0xC2

/*
 * K32 category - these are the functions found in the k32 directory.
 */
#define K32_ALLOCMEMEX          0x01
#define K32_QUERYOTES           0x02
#define K32_QUERYOPTIONSSTATUS  0x03
#define K32_SETOPTIONS          0x04
#define K32_PROCESSREADWRITE    0x05
#define K32_HANDLESYSTEMEVENT   0x06
#define K32_QUERYSYSTEMMEMINFO  0x07
#define K32_QUERYCALLGATE       0x08
#define K32_LASTIOCTLFUNCTION   K32_QUERYCALLGATE


/*
 * Elf category
 */
#define ELF_DUMMY               0x01


/*
 * DosAllocMemEx - Extra (bsememf.h) flag definitions.
 */
#define OBJ_ALIGNDEFAULT        0x00000000UL
#define OBJ_ALIGN64K            0x10000000UL
#define OBJ_ALIGNPAGE           0x20000000UL
#define OBJ_ALIGNMASK           0x30000000UL
#define OBJ_LOCATION            0x80000000UL
#ifndef OBJ_SELMAPALL
#define OBJ_SELMAPALL           0x00000800UL    /* This isn't ours. It just a dropout from bsememf.h */
#endif
#ifndef OBJ_ANY
#define OBJ_ANY                 0x00000400UL    /* This isn't ours. It just no present in all toolkits. */
#endif

/*
 * System event Identifiers.
 */
#define K32_SYSEVENT_SM_MOUSE               0x00    /* dh SendEvent - Session Manager (mouse)    */
#define K32_SYSEVENT_CTRL_BREAK             0x01    /* dh SendEvent - Ctrl-Break                 */
#define K32_SYSEVENT_CTRL_C                 0x02    /* dh SendEvent - Ctrl-C                     */
#define K32_SYSEVENT_CTRL_SCREEN_LOCK       0x03    /* dh SendEvent - Ctrl-ScrollLock            */
#define K32_SYSEVENT_CTRL_PRINT_SCREEN      0x04    /* dh SendEvent - Ctrl-PrtSc                 */
#define K32_SYSEVENT_SHIFT_PRINT_SCREEN     0x05    /* dh SendEvent - Shift-PrtSc                */
#define K32_SYSEVENT_SM_KEYBOARD            0x06    /* dh SendEvent - Session Manager (keyboard) */
#define K32_SYSEVENT_SM_CTRL_ALT_DELETE     0x07    /* dh SendEvent - Ctrl-Alt-Del               */
#define K32_SYSEVENT_HOT_PLUG               0x08    /* dh SendEvent - Keyboard Hot Plug/Reset    */
#define K32_SYSEVENT_POWER                  0x09    /* dh SendEvent - Power suspend event        */
#define K32_SYSEVENT_POWEROFF               0x0a    /* dh SendEvent - Power off event            */
#define K32_SYSEVENT_CTRL_ALT_2xNUMLOCK     0x0b    /* VectorSDF    - System Dump                */
#define K32_SYSEVENT_LAST                   K32_SYSEVENT_CTRL_ALT_2xNUMLOCK

/*
 * System Memory Info flags.
 */
#define K32_SYSMEMINFO_ALL                  0       /* Everything is queried. */
#define K32_SYSMEMINFO_SWAPFILE             0x01    /* Swapfile stuff is queried. */
#define K32_SYSMEMINFO_PAGING               0x02    /* Paging and Physical memory stuff is queried. */
#define K32_SYSMEMINFO_VM                   0x04    /* Virtual memory stuff is all queried. */


/* NOINC */
#ifndef INCL_16                         /* We don't need this in 16-bit code. */
/* INC */

/*******************************************************************************
*   Assembly Fixes                                                             *
*******************************************************************************/
/*ASM
BOOL struc
        dd ?
BOOL ends

PID struc
        dd ?
PID ends

HMODULE struc
        dd ?
HMODULE ends

HEV struc
        dd ?
HEV ends

PVOID struc
        dd ?
PVOID ends

PPVOID struc
        dd ?
PPVOID ends

PUSHORT struc
        dd ?
PUSHORT ends

CCHMAXPATH EQU 260

*/  /* end of assembly */


/*******************************************************************************
*   Structures and Typedefs                                                    *
*******************************************************************************/
#pragma pack(1)

/*
 * Object Table Entry buffer.
 */
struct _QueryOTE
{
    ULONG   ote_size;                   /* Object virtual size */
    ULONG   ote_base;                   /* Object base virtual address */
    ULONG   ote_flags;                  /* Attribute flags */
    ULONG   ote_pagemap;                /* Object page map index */
    ULONG   ote_mapsize;                /* Num of entries in obj page map */
  /*ULONG   ote_reserved;*/
    USHORT  ote_sel;                    /* Object Selector */
    USHORT  ote_hob;                    /* Object Handle */
};
typedef struct _QueryOTE    QOTE;
typedef QOTE     *          PQOTE;


typedef struct _QueryOTEBuffer
{
    ULONG   cOTEs;                      /* Number of entries in aOTE. */
    QOTE    aOTE[1];                    /* Array of OTEs. */
} QOTEBUFFER;
typedef QOTEBUFFER *PQOTEBUFFER;


/*
 * Options struct.
 *
 * (The function of these members are described in options.h in
 *  the src\win32k\include directory.)
 * All members of this struct (except cb ofcourse) is changable.
 */
typedef struct _K32Options
{
    ULONG   cb;

    /** @cat logging options */
    USHORT      usCom;                  /* Output port no. */
    ULONG       fLogging;               /* Logging. */

    /** @cat Options affecting the generated LX executables */
    ULONG       fPE;                    /* Flags set the type of conversion. */
    ULONG       fPEOneObject;           /* All in one object. */
    ULONG       ulInfoLevel;            /* Pe2Lx InfoLevel. */

    /** @cat Options affecting the generated ELF executables */
    ULONG       fElf;                   /* Elf flags. */

    /** @cat Options affecting the UNIX script executables */
    ULONG       fUNIXScript;            /* UNIX script flags. */

    /** @cat Options affecting the REXX script executables */
    ULONG       fREXXScript;            /* REXX script flags. */

    /** @cat Options affecting the JAVA executables */
    ULONG       fJava;                  /* Java flags. */

    /** @cat Options affecting the  executables */
    ULONG       fNoLoader;              /* No loader stuff. !FIXME! We should import / functions even if this flag is set!!! */

    /** @cat Options affecting the behaviour changes in the OS/2 loader */
    ULONG       fDllFixes;              /* Enables the long DLL name and non .DLL extention fixes. */
    ULONG       fExeFixes;              /* Enables EXE files to export entry points. */
    ULONG       fForcePreload;          /* Forces the loader to preload executable images. Handy for ICAT Ring-3 debugging. */
    ULONG       fApiEnh;                /* Enables the API enhancements */

    /** @cat Options affecting the heap. */
    ULONG       cbSwpHeapMax;           /* Maximum heapsize. */
    ULONG       cbResHeapMax;           /* Maxiumem residentheapsize. */
} K32OPTIONS;
typedef K32OPTIONS  *PK32OPTIONS;


/*
 * Status struct.
 *
 */
typedef struct _K32Status
{
    ULONG   cb;

    /** @cat Options status. */
    ULONG       fQuiet;                 /* Quiet initialization. */

    /** @cat Kernel status. */
    ULONG       fKernel;                /* Smp or uni kernel. */
    ULONG       ulBuild;                /* Kernel build. */
    USHORT      usVerMajor;             /* OS/2 major ver - 20 */
    USHORT      usVerMinor;             /* OS/2 minor ver - 30,40 */

    /** @cat Heap status. */
    ULONG       cbSwpHeapInit;          /* Initial heapsize. */
    ULONG       cbSwpHeapFree;          /* Amount of used space. */
    ULONG       cbSwpHeapUsed;          /* Amount of free space reserved. */
    ULONG       cbSwpHeapSize;          /* Amount of memory used by the heap free and used++. */
    ULONG       cSwpBlocksUsed;         /* Count of used blocks. */
    ULONG       cSwpBlocksFree;         /* Count of free blocks. */

    ULONG       cbResHeapInit;          /* Initial heapsize. */
    ULONG       cbResHeapFree;          /* Amount of free space reserved. */
    ULONG       cbResHeapUsed;          /* Amount of used space. */
    ULONG       cbResHeapSize;          /* Amount of memory used by the heap free and used++. */
    ULONG       cResBlocksUsed;         /* Count of used blocks. */
    ULONG       cResBlocksFree;         /* Count of free blocks. */

    /** @cat Win32k build, version and init stuff */
    CHAR        szBuildDate[12];        /* Date of the win32k build. (Sep 02 2000) */
    CHAR        szBuildTime[9];         /* Time of the win32k build. (11:44:21) */
    ULONG       ulVersion;              /* Win32k version */
    CHAR        szSymFile[CCHMAXPATH];  /* The name of the symbol file or sym database. */

    /** @cat Statistics */
    ULONG       cPe2LxModules;          /* Number of Pe2Lx modules currently loaded. */
    ULONG       cElf2LxModules;         /* Number of Elf2Lx modules currently loaded. */
    /*...*/
} K32STATUS;
typedef K32STATUS * PK32STATUS;


/*
 * Memory information struct.
 */
typedef struct _k32SystemMemInfo
{
    ULONG       cb;                     /* Size of this structure - must be set. */
                                        /* This will be used to distinguish futher versions. */
    ULONG       flFlags;                /* This is used to limit amount of information collected - must be set. (K32_SYSMEMINFO_*) */
                                        /* Some conciderations.
                                         *    - VM uses worker functions which traverses internal lists.
                                         *    - Page and physical memory traverses one internal structure (PGPhysAvail())
                                         *    - Swap File only accesses variables.
                                         */

    /* SwapFile Info */
    BOOL        fSwapFile;              /* Swap File: Swapping enabled or disabled. (SMswapping) */
                                        /*            (The SwapFile data below is valid when swapping is enbled!) */
    ULONG       cbSwapFileSize;         /* Swap File: Current size. (smFileSize<<PAGESIZE) */
    ULONG       cbSwapFileAvail;        /* Swap File: Available within current file. ((smFileSize-smcBrokenDF-SMcDFInuse)<<PAGESIZE) */
    ULONG       cbSwapFileUsed;         /* Swap File: Used within current file. (SMcDFInuse<<PAGESIZE) */
    ULONG       cbSwapFileMinFree;      /* Swap File: Addjusted min free on swap volume. (SMMinFree<<PAGESHIFT) */
    ULONG       cbSwapFileCFGMinFree;   /* Swap File: Configured min free on swap volume. ((SMCFGMinFree<<PAGESHIFT)/4) */
    ULONG       cbSwapFileCFGSwapSize;  /* Swap File: Configured initial swap file size. ((SMCFGSwapSize<<PAGESHIFT)/4) */
    ULONG       cSwapFileBrokenDF;      /* Swap File: Number of broken disk frames (DF). (smcBrokenDF) */
    ULONG       cSwapFileGrowFails;     /* Swap File: Number of times growoperation has failed. (smcGrowFails) */
    ULONG       cSwapFileInMemFile;     /* Swap File: Number of pages in the "In-Memory-swapFile". (SMcInMemFile) */
                                        /*            These pages are not counted into the SMcDFInuse count I think. */

    /* Physical Memory Info */
    ULONG       cbPhysSize;             /* Physical memory: total (bytes). (pgPhysPages<<PAGESHIFT) */
    ULONG       cbPhysAvail;            /* Physical memory: available (bytes). (PGPhysAvail()) */
    ULONG       cbPhysUsed;             /* Physical memory: used (bytes). (PGPhysPresent()<<PAGESHIFT) */

    /* Other paging info */
    BOOL        fPagingSwapEnabled;     /* Paging: TRUE when swapping is enabled, else false. (PGSwapEnabled) */
    ULONG       cPagingPageFaults;      /* Paging: Number of pagefaults which have occurred since boot. (pgcPageFaults) */
    ULONG       cPagingPageFaultsActive;/* Paging: Number of pagefaults currently being processed. (pgcPageFaultsActive) */
    ULONG       cPagingPhysPages;       /* Paging: Count of physical pages. (hope this is correct) (pgPhysPages) */
    ULONG       ulPagingPhysMax;        /* Paging: Top of physical memory (physical page number) (pgPhysMax) */
    ULONG       cPagingResidentPages;   /* Paging: Count of resident pages. (pgResidentPages) */
    ULONG       cPagingSwappablePages;  /* Paging: Count of swappable pages which is currently present in memory. (pgSwappablePages) */
    ULONG       cPagingDiscardableInmem;/* Paging: Count of discardable pages which is currently present in memory. (pgDiscardableInmem) */
    ULONG       cPagingDiscardablePages;/* Paging: Count of discardable pages allocated. (pgDiscardablePages) */

    /* Virtual Memory manager info. */
    ULONG       ulAddressLimit;         /* VM: Current user virtual address limit - use this for high arena check. (VirtualAddressLimit / 0x20000000) */
    ULONG       ulVMArenaPrivMax;       /* VM: Current highest address in the private arena. (vmRecalcShrBound()) */
    ULONG       ulVMArenaSharedMin;     /* VM: Current lowest address in the shared arena. (ahvmShr) */
    ULONG       ulVMArenaSharedMax;     /* VM: Current highest address in the shared arena. (ahvmShr) */
    ULONG       ulVMArenaSystemMin;     /* VM: Current lowest address in the system arena. (ahvmhSys) */
    ULONG       ulVMArenaSystemMax;     /* VM: Current highest address in the system arena. (ahvmhSys) */
    ULONG       ulVMArenaHighPrivMax;   /* VM: Current highest address in the high private arena - aurora/smp only. (vmRecalcShrBound) */
    ULONG       ulVMArenaHighSharedMin; /* VM: Current lowest address in the high shared arena - aurora/smp only. (ahvmhShr) */
    ULONG       ulVMArenaHighSharedMax; /* VM: Current highest address in the high shared arena - aurora/smp only. (ahvmhShr) */

} K32SYSTEMMEMINFO;
typedef K32SYSTEMMEMINFO *  PK32SYSTEMMEMINFO;


/*
 * K32 category parameter structs
 * ---
 * First member is allways a K32HDR struct called hdr.
 * The rest of it should be exactly like the parameter list for the API call.
 */
struct _k32Header                       /* Common parameter header. */
{
    ULONG       cb;                     /* Size of the parameters struct. */
    ULONG       rc;                     /* Return code. */
};
typedef struct _k32Header   K32HDR;
typedef K32HDR *            PK32HDR;


struct _k32AllocMemEx
{
    K32HDR      hdr;                    /* Common parameter header */
    PPVOID      ppv;                    /* Pointer to pointer to the allocated memory block */
                                        /* On input it (*ppv) may hold the suggested  location of the block. */
    ULONG       cb;                     /* Blocksize (bytes) */
    ULONG       flFlags;                /* Flags (equal to DosAllocMem flags) */
    ULONG       ulCS;                   /* Call CS */
    ULONG       ulEIP;                  /* Call EIP */
};
typedef struct _k32AllocMemEx   K32ALLOCMEMEX;
typedef K32ALLOCMEMEX *         PK32ALLOCMEMEX;


struct _k32QueryOTEs
{
    K32HDR      hdr;                    /* Common parameter header */
    HMODULE     hMTE;                   /* Module handle. */
    PQOTEBUFFER pQOte;                  /* Pointer to output buffer. */
    ULONG       cbQOte;                 /* Size of the buffer pointed to by pQOte  */
};
typedef struct _k32QueryOTEs    K32QUERYOTES;
typedef K32QUERYOTES *          PK32QUERYOTES;


struct _k32QueryOptionsStatus
{
    K32HDR      hdr;                    /* Common parameter header */
    PK32OPTIONS pOptions;               /* Pointer to option struct. (NULL allowed) */
    PK32STATUS  pStatus;                /* Pointer to status struct. (NULL allowed) */
};
typedef struct _k32QueryOptionsStatus   K32QUERYOPTIONSSTATUS;
typedef K32QUERYOPTIONSSTATUS *         PK32QUERYOPTIONSSTATUS;


struct _k32SetOptions
{
    K32HDR      hdr;                    /* Common parameter header */
    PK32OPTIONS pOptions;               /* Pointer to option struct. (NULL allowed) */
};
typedef struct _k32SetOptions           K32SETOPTIONS;
typedef K32SETOPTIONS *                 PK32SETOPTIONS;


struct _k32ProcessReadWrite
{
    K32HDR      hdr;                    /* Common parameter header */
    PID         pid;                    /* Process ID of the process to access memory in. */
    ULONG       cb;                     /* Number of bytes to read or write. */
    PVOID       pvSource;               /* Pointer to source data. */
    PVOID       pvTarget;               /* Pointer to target area. */
    BOOL        fRead;                  /* TRUE:   pvSource is within pid while pvTarget is ours. */
                                        /* FALSE:  pvTarget is within pid while pvSource is ours. */
};
typedef struct _k32ProcessReadWrite     K32PROCESSREADWRITE;
typedef K32PROCESSREADWRITE *           PK32PROCESSREADWRITE;


struct _k32HandleSystemEvent
{
    K32HDR      hdr;                    /* Common parameter header */
    ULONG       ulEvent;                /* Event identifier. One of the K32_SYSEVENT_ defines. */
    HEV         hev;                    /* Handle of shared event semaphore which should be */
                                        /* posted when the the requested event occurs.      */
    BOOL        fHandle;                /* Action flag. */
                                        /* TRUE:  Take control of the event. */
                                        /* FALSE: Give control back to the OS of this event. (hev must match the current handle!) */
};
typedef struct _k32HandleSystemEvent    K32HANDLESYSTEMEVENT;
typedef K32HANDLESYSTEMEVENT *          PK32HANDLESYSTEMEVENT;


struct _k32QuerySystemMemInfo
{
    K32HDR      hdr;                    /* Common parameter header */
    PK32SYSTEMMEMINFO   pMemInfo;       /* Pointer to system memory info structure with cb set. */
                                        /* The other members will be filled on successful return. */
};
typedef struct _k32QuerySystemMemInfo   K32QUERYSYSTEMMEMINFO;
typedef K32QUERYSYSTEMMEMINFO *         PK32QUERYSYSTEMMEMINFO;


struct _k32QueryCallGate
{
    K32HDR      hdr;                    /* Common parameter header */
    PUSHORT     pusCGSelector;          /* Pointer to variable where the callgate selector */
                                        /* is to be stored. */
};
typedef struct _k32QueryCallGate        K32QUERYCALLGATE;
typedef struct _k32QueryCallGate *      PK32QUERYCALLGATE;

#pragma pack()

/* NOINC */
#ifndef NO_WIN32K_LIB_FUNCTIONS
/*******************************************************************************
*   External Functions                                                         *
*******************************************************************************/
/* Win32k APIs */
APIRET APIENTRY  libWin32kInit(void);
APIRET APIENTRY  libWin32kTerm(void);
BOOL   APIENTRY  libWin32kInstalled(void);
APIRET APIENTRY  libWin32kQueryOptionsStatus(PK32OPTIONS pOptions, PK32STATUS pStatus);
APIRET APIENTRY  libWin32kSetOptions(PK32OPTIONS pOptions);

/* "Extra OS2 APIs" */
APIRET APIENTRY  DosAllocMemEx(PPVOID ppv, ULONG cb, ULONG flag);
APIRET APIENTRY  W32kQueryOTEs(HMODULE hMTE, PQOTEBUFFER pQOte, ULONG cbQOte);
APIRET APIENTRY  W32kQuerySystemMemInfo(PK32SYSTEMMEMINFO pMemInfo);
APIRET APIENTRY  W32kProcessReadWrite(PID pid, ULONG cb, PVOID pvSource, PVOID pvTarget, BOOL fRead);
APIRET APIENTRY  W32kHandleSystemEvent(ULONG ulEvent, HEV hev, BOOL fHandle);

/* Helper function */
USHORT APIENTRY  libHelperGetCS(void);

#endif
/* INC */

/* NOINC */
#endif /* !defined(INCL_16) */
#endif
/* INC */
