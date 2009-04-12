/* $Id: ldr.h,v 1.2 2002/06/11 00:29:00 bird Exp $
 *
 * Loader structures - from kKrnlLib (win32k)
 *
 * Copyright (c) 2002 knut st. osmundsen (bird@anduin.net)
 *
 * GPL
 *
 */

#ifndef __LDR_H__
#define __LDR_H__

/*******************************************************************************
*   Structures and Typedefs                                                    *
*******************************************************************************/
struct OTE
{
    unsigned long    ote_size;             /* Object virtual size */
    unsigned long    ote_base;             /* Object base virtual address */
    unsigned long    ote_flags;            /* Attribute flags */
    unsigned long    ote_pagemap;          /* Object page map index */
    unsigned long    ote_mapsize;          /* Num of entries in obj page map */
    unsigned short   ote_sel;              /* Object Selector */
    unsigned short   ote_hob;              /* Object Handle */
};


/* ote_flags */
#if !defined(OBJREAD)
  #define OBJREAD       0x00000001L     /* Readable Object */
  #define OBJWRITE      0x00000002L     /* Writeable Object */
  #define OBJEXEC       0x00000004L     /* Executable Object */
  #define OBJRSRC       0x00000008L     /* Resource Object */
  #define OBJDISCARD    0x00000010L     /* Object is Discardable */
  #define OBJSHARED     0x00000020L     /* Object is Shared */
  #define OBJPRELOAD    0x00000040L     /* Object has preload pages */
  #define OBJINVALID    0x00000080L     /* Object has invalid pages */
#endif
#define OBJZEROFIL      0x00000100L     /* Object has zero-filled pages */
#if !defined(OBJRESIDENT)
  #define OBJRESIDENT   0x00000200L     /* Object is resident */
  #define OBJCONTIG     0x00000300L     /* Object is resident and contiguous */
  #define OBJDYNAMIC    0x00000400L     /* Object is permanent and long locable */
  #define OBJTYPEMASK   0x00000700L     /* Object type mask */
  #define OBJALIAS16    0x00001000L     /* 16:16 alias required */
  #define OBJBIGDEF     0x00002000L     /* Big/Default bit setting */
  #define OBJCONFORM    0x00004000L     /* Object is conforming for code */
  #define OBJIOPL       0x00008000L     /* Object I/O privilege level */
#endif
#define OBJMADEPRIV     0x40000000L     /* Object is made private for debug (now obsolete) */
#define OBJALLOC        0x80000000L     /* Object is allocated used by loader */






/*************************/
/* warp 3.0 AS SMTEs     */
/* Swap ModuleTableEntry */
/*************************/
struct SMTE
{
    /* lxheader fields */
        /*- magic,border,worder,level,cpu,os,ver,mflags */
    unsigned long   smte_mpages;        /* 00  Module # pages */
    unsigned long   smte_startobj;      /* 04  Object # for instruction */
    unsigned long   smte_eip;           /* 08  Extended instruction pointer */
    unsigned long   smte_stackobj;      /* 0c  Object # for stack pointer */
    unsigned long   smte_esp;           /* 10  Extended stack pointer */
        /*- pagesize*/
    unsigned long   smte_pageshift;      /* 14  Page alignment shift in .EXE */
    unsigned long   smte_fixupsize;     /* 18  Fixup section size */
        /*- fixupsum,ldrsize,ldrsum*/
    struct OTE FAR *    smte_objtab;        /* 1c  Object table offset - POINTER */
    unsigned long   smte_objcnt;        /* 20  Number of objects in module */
    unsigned long   smte_objmap;        /* 24  Object page map offset - POINTER */
    unsigned long   smte_itermap;       /* 28  Object iterated data map offset */
    unsigned long   smte_rsrctab;       /* 2c  Offset of Resource Table */
    unsigned long   smte_rsrccnt;       /* 30  Number of resource entries */
    unsigned long   smte_restab;        /* 34  Offset of resident name table - POINTER */
    unsigned long   smte_enttab;        /* 38  Offset of Entry Table - POINTER */
    unsigned long   smte_fpagetab;      /* 3c  Offset of Fixup Page Table - POINTER */
    unsigned long   smte_frectab;       /* 40  Offset of Fixup Record Table - POINTER */
    unsigned long   smte_impmod;        /* 44  Offset of Import Module Name Table - POINTER */
        /*- impmodcnt*/
    unsigned long   smte_impproc;       /* 48  Offset of Imp Procedure Name Tab - POINTER */
        /*- pagesum*/
    unsigned long   smte_datapage;      /* 4c  Offset of Enumerated Data Pages */
        /*- preload*/
    unsigned long   smte_nrestab;       /* 50  Offset of Non-resident Names Table */
    unsigned long   smte_cbnrestab;     /* 54  Size of Non-resident Name Table */
        /*- nressum*/
    unsigned long   smte_autods;        /* 58  Object # for automatic data object */
    unsigned long   smte_debuginfo;     /* 5c  Offset of the debugging info */
    unsigned long   smte_debuglen;      /* 60  The len of the debug info in */
        /*- instpreload,instdemand*/
    unsigned long   smte_heapsize;      /* 64  use for converted 16-bit modules */
        /*- res3*/
    /* extra */
    char FAR *      smte_path;          /* 68  full pathname - POINTER */
    unsigned short  smte_semcount;      /* 6c  Count of threads waiting on MTE semaphore. 0 => semaphore is free */
    unsigned short  smte_semowner;      /* 6e  Slot number of the owner of MTE semahore */
    unsigned long   smte_pfilecache;    /* 70  Pointer to file cache for Dos32CacheModule */
    unsigned long   smte_stacksize;     /* 74  Thread 1 Stack size from the exe header */
    unsigned short  smte_alignshift;    /* 78  use for converted 16-bit modules */
    unsigned short  smte_NEexpver;      /* 7a  expver from NE header */
    unsigned short  smte_pathlen;       /* 7c  length of full pathname */
    unsigned short  smte_NEexetype;     /* 7e  exetype from NE header */
    unsigned short  smte_csegpack;      /* 80  count of segs to pack */
};








/********************/
/* warp 3.0 GA MTEs */
/* ModuleTableEntry */
/********************/
struct MTE
{
   unsigned short mte_flags2;
   unsigned short mte_handle;
   struct SMTE FAR * mte_swapmte;/* handle for swapmte */
   struct MTE FAR * mte_link;
   unsigned long  mte_flags1;
   unsigned long  mte_impmodcnt; /* number of entries in Import Module Name Table*/
   unsigned short mte_sfn;       /*"filehandle"*/
   unsigned short mte_usecnt;    /* (.EXE only) - use count */
   char           mte_modname[8];
};


/***********/
/* flags 1 */
/***********/
   #define NOAUTODS            0x00000000   /* No Auto DS exists */
   #define SOLO                0x00000001   /* Auto DS is shared */
   #define INSTANCEDS          0x00000002   /* Auto DS is not shared */
   #define INSTLIBINIT         0x00000004   /* Perinstance Libinit */
   #define GINISETUP           0x00000008   /* Global Init has been setup */
   #define NOINTERNFIXUPS        0x00000010 /* internal fixups in .EXE.DLL applied */
   #define NOEXTERNFIXUPS      0x00000020   /* external fixups in .EXE.DLL applied */
   #define CLASS_PROGRAM       0x00000040   /* Program class */
   #define CLASS_GLOBAL        0x00000080   /* Global class */
   #define CLASS_SPECIFIC      0x000000C0   /* Specific class, as against global */
   #define CLASS_ALL           0x00000000   /* nonspecific class  all modules */
   #define CLASS_MASK          0x000000C0 /* */
   #define MTEPROCESSED        0x00000100   /* MTE being loaded */
   #define USED                0x00000200   /* MTE is referenced  see ldrgc.c */
   #define DOSLIB              0x00000400   /* set if DOSCALL1 */
   #define DOSMOD              0x00000800   /* set if DOSCALLS */
   #define MTE_MEDIAFIXED      0x00001000   /* File Media permits discarding */
   #define LDRINVALID          0x00002000   /* module not loadable */
   #define PROGRAMMOD          0x00000000   /* program module */
   #define DEVDRVMOD           0x00004000   /* device driver module */
   #define LIBRARYMOD          0x00008000   /* DLL module */
   #define VDDMOD              0x00010000   /* VDD module */
   #define MVDMMOD             0x00020000   /* Set if VDD Helper MTE (MVDM.DLL) */
   #define INGRAPH             0x00040000   /* In Module Graph  see ldrgc.c */
   #define GINIDONE            0x00080000   /* Global Init has finished */
   #define MTEADDRALLOCED      0x00100000   /* Allocate specific or not */
   #define FSDMOD              0x00200000   /* FSD MTE */
   #define FSHMOD              0x00400000   /* FS helper MTE */
   #define MTELONGNAMES        0x00800000   /* Module supports longnames */
   #define MTE_MEDIACONTIG     0x01000000   /* File Media contiguous memory req */
   #define MTE_MEDIA16M        0x02000000   /* File Media requires mem below 16M */
   #define MTESWAPONLOAD       0x04000000   /* make code pages swap on load */
   #define MTEPORTHOLE         0x08000000   /* porthole module */
   #define MTEMODPROT          0x10000000   /* Module has shared memory protected */
   #define MTENEWMOD           0x20000000   /* Newly added module */
   #define MTEDLLTERM          0x40000000   /* Gets instance termination */
   #define MTESYMLOADED        0x80000000   /* Set if debugger symbols loaded */


/***********/
/* flags 2 */
/***********/
   #define MTEFORMATMASK         0x0003     /* Module format mask */
   #define MTEFORMATR1           0x0000     /* Module format reserved */
   #define MTEFORMATNE           0x0001     /* Module format NE */
   #define MTEFORMATLX           0x0002     /* Module format LX */
   #define MTEFORMATR2           0x0003     /* Module format reserved */
   #define MTESYSTEMDLL          0x0004     /* DLL exists in system list */
   #define MTELOADORATTACH       0x0008     /* Module under load or attach  for init */
   #define MTECIRCLEREF          0x0010     /* Module circular reference detection */
   #define MTEFREEFIXUPS         0x0020     /* Free system mte's fixup flag d#98488 */
   #define MTEPRELOADED          0x0040     /* MTE Preload completed */
   #define MTEGETMTEDONE         0x0080     /* GetMTE already resolved */
   #define MTEPACKSEGDONE        0x0100     /* Segment packed memory allocated */
   #define MTE20LIELIST          0x0200     /* Name present in version20 lie list */
   #define MTESYSPROCESSED       0x0400     /* System DLL already processed */
   #define MTEDLLONEXTLST        0x1000     /* DLL has term routine on exit list #74177 */


#pragma pack()


#endif /* __LDR_H__ */
