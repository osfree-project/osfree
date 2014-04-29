#ifndef __FSH_H
#define __FSH_H

/*  fsh.h   - FSH_ = fshelper interface declarations */

/*
 *  FSH_DOVOLIO2 omits flag definition
 *
 *  Since we are using C5.1, the function prototypes should be made to
 *  conform with true ANSI standards.  I have converted FSH_ADDSHARE
 *  as an example.
 *
 */

/* XLATOFF */
#if 1
int far pascal
FSH_ADDSHARE(
    char far *,                 /* pName        */
    unsigned short,             /* mode         */
    unsigned short,             /* hVPB         */
    unsigned long far *         /* phShare      */
);
#else
USHORT far pascal
FSH_ADDSHARE(
    PSZ     pName,
    USHORT  mode,
    SHANDLE hVPB,
    LHANDLE phShare
);
#endif

int far pascal
FSH_CANONICALIZE(
    char far *,                 /* pPathName    */
    unsigned short,             /* cbPathBuf    */
    char far *,         /* pPathBuf     */
    char far *                  /* pFlags */
);

int far pascal
FSH_CHECKEANAME(
    unsigned short,             /* level        */
    unsigned long,              /* len of name  */
    char far *                  /* pEAName      */
);

int far pascal
FSH_CRITERROR(
    int,                        /* cbMessage    */
    char far *,                 /* pMessage     */
    int,                        /* nSubs        */
    char far *,                 /* pSubs        */
    unsigned short              /* fAllowed     */
);
/* XLATON */

/*  Flags for fAllowed
 */
#define CE_ALLFAIL      0x0001          /*  FAIL allowed                      */
#define CE_ALLABORT     0x0002          /*  ABORT allowed                     */
#define CE_ALLRETRY     0x0004          /*  RETRY allowed                     */
#define CE_ALLIGNORE    0x0008          /*  IGNORE allowed                    */
#define CE_ALLACK       0x0010          /*  ACK allowed                       */

/*  Return values from FSH_CRITERR
 */
#define CE_RETIGNORE    0x0000          /*  User said IGNORE                  */
#define CE_RETRETRY     0x0001          /*  User said RETRY                   */
#define CE_RETFAIL      0x0003          /*  User said FAIL/ABORT              */
#define CE_RETACK       0x0004          /*  User said continue                */

/* XLATOFF */
int far pascal
FSH_DEVIOCTL(
    unsigned short,             /* FSDRaisedFlag */
    unsigned long,              /* hDev         */
    unsigned short,             /* sfn          */
    unsigned short,             /* cat          */
    unsigned short,             /* func         */
    char far *,                 /* pParm        */
    unsigned short,             /* cbParm       */
    char far *,                 /* pData        */
    unsigned short              /* cbData       */
);

int far pascal
FSH_DOVOLIO(
    unsigned short,             /* operation    */
    unsigned short,             /* fAllowed     */
    unsigned short,             /* hVPB         */
    char far *,                 /* pData        */
    unsigned short far *,       /* pcSec        */
    unsigned long               /* iSec         */
);
/* XLATON */

/*  Flags for operation
 */
#define DVIO_OPREAD     0x0000          /*  no bit on => readi                */
#define DVIO_OPWRITE    0x0001          /*  ON => write else read             */
#define DVIO_OPBYPASS   0x0002          /*  ON => cache bypass else no bypass */
#define DVIO_OPVERIFY   0x0004          /*  ON => verify after write          */
#define DVIO_OPHARDERR  0x0008          /*  ON => return hard errors directly */
#define DVIO_OPWRTHRU   0x0010          /*  ON => write thru                  */
#define DVIO_OPNCACHE   0x0020          /*  ON => don't cache data            */
#define DVIO_OPRESMEM   0x0040          /*  ON => don't lock this memory      */

/*  Flags for fAllowed
 */
#define DVIO_ALLFAIL    0x0001          /*  FAIL allowed                      */
#define DVIO_ALLABORT   0x0002          /*  ABORT allowed                     */
#define DVIO_ALLRETRY   0x0004          /*  RETRY allowed                     */
#define DVIO_ALLIGNORE  0x0008          /*  IGNORE allowed                    */
#define DVIO_ALLACK     0x0010          /*  ACK allowed                       */

/* XLATOFF */
int far pascal
FSH_DOVOLIO2(
    unsigned long,              /* hDev         */
    unsigned short,             /* sfn          */
    unsigned short,             /* cat          */
    unsigned short,             /* func         */
    char far *,                 /* pParm        */
    unsigned short,             /* cbParm       */
    char far *,                 /* pData        */
    unsigned short              /* cbData       */
);

int far pascal
FSH_FINDCHAR(
    unsigned short,             /* nChars       */
    char far *,                 /* pChars       */
    char far * far *            /* ppStr        */
);

int far pascal
FSH_FINDDUPHVPB(
    unsigned short,             /* hVPB         */
    unsigned short far *        /* pHVPB        */
);

int far pascal
FSH_FLUSHBUF(
    unsigned short,             /* hVPB         */
    unsigned short              /* fDiscard     */
);
/* XLATON */

/*  fDiscard values
 */
#define FB_DISCNONE     0x0000          /*  Do not discard buffers            */
#define FB_DISCCLEAN    0x0001          /*  Discard clean buffers             */

/* XLATOFF */
int far pascal
FSH_FORCENOSWAP(
    unsigned short              /* sel          */
);

int far pascal FSH_EXTENDTIMESLICE(void);

int far pascal FSH_GETPRIORITY(void);

int far pascal FSH_IOBOOST(void);

/* XLATON */

/*  Flags for fPreRead
 */
#define GB_PRNOREAD     0x0001          /*  ON => no preread occurs           */

/* XLATOFF */
int far pascal
FSH_GETOVERLAPBUF(
    unsigned short,             /* hVPB         */
    unsigned long,              /* iSec         */
    unsigned long,              /* iSec         */
    unsigned long far *,        /* pisecBuf     */
    char far * far *            /* ppBuf        */
);

int far pascal
FSH_GETVOLPARM(
    unsigned short,             /* hVPB         */
    struct vpfsi far * far *,   /* ppVPBfsi     */
    struct vpfsd far * far *    /* ppVPBfsd     */
);

int far pascal
FSH_INTERR(
    char far *,                 /* pMsg         */
    unsigned short              /* cbMsg        */
);

int far pascal
FSH_ISCURDIRPREFIX(
    char far *                  /* pName        */
);

void far pascal
FSH_LOADCHAR(
    char far * far *,           /* ppStr        */
    unsigned short far *        /* pChar        */
);

void far pascal
FSH_PREVCHAR(
    char far *,                 /* pBeg         */
    char far * far *            /* ppStr        */
);

int far pascal
FSH_PROBEBUF(
    unsigned short,             /* operation    */
    char far *,                 /* pData        */
    unsigned short              /* cbData       */
);
/* XLATON */

/*  Values for operation
 */
#define PB_OPREAD   0x0000              /*  Check for read access             */
#define PB_OPWRITE  0x0001              /*  Check for write access            */

/* XLATOFF */

int far pascal
FSH_QUERYSERVERTHREAD( );

int far pascal
FSH_QUERYOPLOCK( );

int far pascal
FSH_QSYSINFO(
    unsigned short,             /* index        */
    char far *,                 /* pData        */
    unsigned short              /* cbData       */
);
/* XLATON */

/* Values for index
 */
#define QSI_SECSIZE     1       /* index to query max sector size */
#define QSI_PROCID      2       /* index to query PID,UserID and Currentpdb */
#define QSI_THREADNO    3       /* index to query abs.thread no */
#define QSI_VERIFY      4       /* index to query per-process verify */

/* XLATOFF */
int far pascal
FSH_NAMEFROMSFN(
    unsigned short,             /* sfn          */
    char far *,                 /* pName        */
    unsigned short far *        /* pcbName      */
);

int far pascal
FSH_REGISTERPERFCTRS(
    char far *,                 /* pDataBlk     */
    char far *,                 /* pTextBlk     */
    unsigned short              /* fsFlags      */
);
/* XLATON */

/*  Flags for fsFlags
 */
#define RPC_16BIT       0x0000          /*  16-bit FSH interface              */
#define RPC_32BIT       0x0001          /*  32-bit FSH interface              */

/* XLATOFF */

int far pascal
FSH_REMOVESHARE(
    unsigned long               /* hShare       */
);

int far pascal
FSH_SEGALLOC(
    unsigned short,             /* flags        */
    unsigned long,              /* cbSeg        */
    unsigned short far *        /* pSel         */
);
/* XLATON */

/*  Fields for flags
 */
#define SA_FLDT         0x0001          /*  ON => alloc LDT else GDT          */
#define SA_FSWAP        0x0002          /*  ON => swappable memory            */

#define SA_FRINGMASK    0x6000          /*  mask for isolating ring           */
#define SA_FRING0       0x0000          /*  ring 0                            */
#define SA_FRING1       0x2000          /*  ring 1                            */
#define SA_FRING2       0x4000          /*  ring 2                            */
#define SA_FRING3       0x6000          /*  ring 3                            */


/* XLATOFF */
int far pascal
FSH_SEGFREE(
    unsigned short              /* sel          */
);

int far pascal
FSH_SEGREALLOC(
    unsigned short,             /* sel          */
    unsigned long               /* cbSeg        */
);
/* XLATON */


/*  Timeout equates for all semaphore operations
 */
#define TO_INFINITE     0xFFFFFFFFL
#define TO_NOWAIT       0x00000000L

/* XLATOFF */
int far pascal
FSH_SEMCLEAR(
    void far *                  /* pSem         */
);

/* XLATOFF */
int far pascal
FSH_IOSEMCLEAR(
    void far *                  /* pSem         */
);

int far pascal
FSH_SEMREQUEST(
    void far *,                 /* pSem         */
    unsigned long               /* cmsTimeout   */
);

int far pascal
FSH_SEMSET(
    void far *                  /* pSem         */
);

int far pascal
FSH_SEMSETWAIT(
    void far *,                 /* pSem         */
    unsigned long               /* cmsTimeout   */
);

int far pascal
FSH_SEMWAIT(
    void far *,                 /* pSem         */
    unsigned long               /* cmsTimeout   */
);

int far pascal
FSH_SETVOLUME(
    unsigned short,             /* hVPB         */
    unsigned long               /* fControl     */
);

int far pascal
FSH_STORECHAR(
    unsigned short,             /* chDBCS       */
    char far * far *            /* ppStr        */
);

int far pascal
FSH_UPPERCASE(
    char far *,                 /* pName        */
    unsigned short,             /* cbPathBuf    */
    char far *                  /* pPathBuf     */
);

int far pascal
FSH_WILDMATCH(
    char far *,                 /* pPat         */
    char far *                  /* pStr         */
);

int far pascal
FSH_YIELD(void);

int far pascal
FSH_CALLDRIVER(
    void far *,                 /* pPkt         */
    unsigned short,             /* hVPB         */
    unsigned short              /* fControl     */
);

int far pascal
MFSH_DOVOLIO(
    char far *,                 /* Data         */
    unsigned short far *,       /* cSec         */
    unsigned long               /* iSec         */
);

int far pascal
MFSH_INTERR(
    char far *,                 /* Msg          */
    unsigned short              /* cbMsg        */
);

int far pascal
MFSH_SEGALLOC(
    unsigned short,             /* Flag         */
    unsigned long,              /* cbSeg        */
    unsigned short far *        /* Sel          */
);

int far pascal
MFSH_SEGFREE(
    unsigned short              /* Sel          */
);

int far pascal
MFSH_SEGREALLOC(
    unsigned short,             /* Sel          */
    unsigned long               /* cbSeg        */
);

int far pascal
MFSH_SETBOOTDRIVE(
    unsigned short              /* 0-based drive # (C=2) */
);

int far pascal
MFSH_CALLRM(
    unsigned long far *         /* Proc         */
);

int far pascal
MFSH_LOCK(
    unsigned short,             /* Sel          */
    unsigned long far *         /* Handle       */
);

int far pascal
MFSH_PHYSTOVIRT(
    unsigned long,              /* Addr         */
    unsigned short,             /* Len          */
    unsigned short far *        /* Sel          */
);

int far pascal
MFSH_UNLOCK(
    unsigned long               /* Handle       */
);

int far pascal
MFSH_UNPHYSTOVIRT(
    unsigned short              /* Sel          */
);

int far pascal
MFSH_VIRT2PHYS(
    unsigned long,              /* VirtAddr     */
    unsigned long far *         /* PhysAddr     */
);
/* XLATON */

#endif /* __FSH_H */

