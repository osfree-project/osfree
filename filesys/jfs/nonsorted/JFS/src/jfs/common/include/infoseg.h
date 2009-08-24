/* $Id: infoseg.h,v 1.1.1.1 2003/05/21 13:35:38 pasha Exp $ */

/*static char *SCCSID = "h/infoseg.h, h, r207, 8s.162 92/06/18";*/
/*static char *SCCSID = "h/infoseg.h, h, r207, 8s.162 92/06/18";*/
/*
 *      InfoSeg Data Structures
 */

/*
 * The structure below defines the content and organization of the system
 * information segment (InfoSeg).  The actual table is statically defined in
 * SDATA.ASM.  Ring 0, read/write access is obtained by the clock device
 * driver using the DevHlp GetDOSVar function.  (GetDOSVar returns a ring 0,
 * read-only selector to all other requestors.)
 *
 * In order to prevent an errant process from destroying the infoseg, two
 * identical global infosegs are maintained.  One is in the tiled shared
 * arena and is accessible in user mode (and therefore can potentially be
 * overwritten from ring 2), and the other is in the system arena and is
 * accessible only in kernel mode.  All kernel code (except the clock driver)
 * is responsible for updating BOTH copies of the infoseg.  The copy kept
 * in the system arena is addressable as DOSGROUP:SISData, and the copy
 * in the shared arena is addressable via a system arena alias.  16:16 and
 * 0:32 pointers to the alias are stored in _Sis2.
 */

struct InfoSegGDT {

/* Time (offset 0x00) */

unsigned long   SIS_BigTime;    /* Time from 1-1-1970 in seconds */
unsigned long   SIS_MsCount;    /* Freerunning milliseconds counter */
unsigned char   SIS_HrsTime;    /* Hours */
unsigned char   SIS_MinTime;    /* Minutes */
unsigned char   SIS_SecTime;    /* Seconds */
unsigned char   SIS_HunTime;    /* Hundredths of seconds */
unsigned short  SIS_TimeZone;   /* Timezone in min from GMT (Set to EST) */
unsigned short  SIS_ClkIntrvl;  /* Timer interval (units=0.0001 secs) */

/* Date (offset 0x10) */

unsigned char   SIS_DayDate;    /* Day-of-month (1-31) */
unsigned char   SIS_MonDate;    /* Month (1-12) */
unsigned short  SIS_YrsDate;    /* Year (>= 1980) */
unsigned char   SIS_DOWDate;    /* Day-of-week (1-1-80 = Tues = 3) */

/* Version (offset 0x15) */

unsigned char   SIS_VerMajor;   /* Major version number */
unsigned char   SIS_VerMinor;   /* Minor version number */
unsigned char   SIS_RevLettr;   /* Revision letter */

/* System Status (offset 0x18) */

/* XLATOFF */
#ifdef  OLDVER
unsigned char   CurScrnGrp;     /* Fgnd screen group # */
#else
/* XLATON */
unsigned char   SIS_CurScrnGrp; /* Fgnd screen group # */
/* XLATOFF */
#endif
/* XLATON */
unsigned char   SIS_MaxScrnGrp; /* Maximum number of screen groups */
unsigned char   SIS_HugeShfCnt; /* Shift count for huge segments */
unsigned char   SIS_ProtMdOnly; /* Protect-mode-only indicator */
unsigned short  SIS_FgndPID;    /* Foreground process ID */

/* Scheduler Parms (offset 0x1E) */

unsigned char   SIS_Dynamic;    /* Dynamic variation flag (1=enabled) */
unsigned char   SIS_MaxWait;    /* Maxwait (seconds) */
unsigned short  SIS_MinSlice;   /* Minimum timeslice (milliseconds) */
unsigned short  SIS_MaxSlice;   /* Maximum timeslice (milliseconds) */

/* Boot Drive (offset 0x24) */

unsigned short  SIS_BootDrv;    /* Drive from which system was booted */

/* RAS Major Event Code Table (offset 0x26) */

unsigned char   SIS_mec_table[32]; /* Table of RAS Major Event Codes (MECs) */

/* Additional Session Data (offset 0x46) */

unsigned char   SIS_MaxVioWinSG;  /* Max. no. of VIO windowable SG's */
unsigned char   SIS_MaxPresMgrSG; /* Max. no. of Presentation Manager SG's */

/* Error logging Information (offset 0x48) */

unsigned short  SIS_SysLog;     /* Error Logging Status */

/* Additional RAS Information (offset 0x4A) */

unsigned short  SIS_MMIOBase;   /* Memory mapped I/O selector */
unsigned long   SIS_MMIOAddr;   /* Memory mapped I/O address  */

/* Additional 2.0 Data (offset 0x50) */

unsigned char   SIS_MaxVDMs;      /* Max. no. of Virtual DOS machines */
unsigned char   SIS_Reserved;

unsigned char   SIS_perf_mec_table[32];

};

#define SIS_LEN         sizeof(struct InfoSegGDT)

/* ASM
SIS_TIMELEN     equ     21      ; size of time and date fields
        .errnz  SIS_BigTime
        .errnz  SIS_VerMajor - SIS_TIMELEN
*/


/* XLATOFF */
/*
 *      The following definitions exist purely for compatibility
 *      with earlier versions of this file.  If new fields are
 *      added to the Global Infoseg, there is no need to add
 *      defines for them to this section.
 */

#define BigTime         SIS_BigTime
#define MsCount         SIS_MsCount
#define HrsTime         SIS_HrsTime
#define MinTime         SIS_MinTime
#define SecTime         SIS_SecTime
#define HunTime         SIS_HunTime
#define TimeZone        SIS_TimeZone
#define ClkIntrvl       SIS_ClkIntrvl
#define DayDate         SIS_DayDate
#define MonDate         SIS_MonDate
#define YrsDate         SIS_YrsDate
#define DOWDate         SIS_DOWDate
#define VerMajor        SIS_VerMajor
#define VerMinor        SIS_VerMinor
#define RevLettr        SIS_RevLettr
#define MaxScrnGrp      SIS_MaxScrnGrp
#define HugeShfCnt      SIS_HugeShfCnt
#define ProtMdOnly      SIS_ProtMdOnly
#define FgndPID         SIS_FgndPID
#define Dynamic         SIS_Dynamic
#define MaxWait         SIS_MaxWait
#define MinSlice        SIS_MinSlice
#define MaxSlice        SIS_MaxSlice
#define BootDrv         SIS_BootDrv
#define mec_table       SIS_mec_table
#define MaxVioWinSG     SIS_MaxVioWinSG
#define MaxPresMgrSG    SIS_MaxPresMgrSG
#define SysLog          SIS_SysLog
#ifdef MMIOPH
#define MMIOBase        SIS_MMIOBase
#endif
/* XLATON */



/*
 *      InfoSeg LDT Data Segment Structure
 *
 * The structure below defines the content and organization of the system
 * information in a special per-process segment to be accessible by the
 * process through the LDT (read-only).
 *
 * As in the global infoseg, two copies of the current processes local
 * infoseg exist, one accessible in both user and kernel mode, the other
 * only in kernel mode.  Kernel code is responsible for updating BOTH copies.
 * Pointers to the local infoseg copy are stored in _Lis2.
 *
 * Note that only the currently running process has an extra copy of the
 * local infoseg.  The copy is done at context switch time.
 */

struct InfoSegLDT {

unsigned short  LIS_CurProcID;  /* Current process ID */
unsigned short  LIS_ParProcID;  /* Process ID of parent */
unsigned short  LIS_CurThrdPri; /* Current thread priority */
unsigned short  LIS_CurThrdID;  /* Current thread ID */
/* XLATOFF */
#ifdef  OLDVER
unsigned short  CurScrnGrp;     /* Screengroup */
#else
/* XLATON */
unsigned short  LIS_CurScrnGrp; /* Screengroup */
/* XLATOFF */
#endif
/* XLATON */
unsigned char   LIS_ProcStatus; /* Process status bits */
unsigned char   LIS_fillbyte1;  /* filler byte */
unsigned short  LIS_Fgnd;       /* Current process is in foreground */
unsigned char   LIS_ProcType;   /* Current process type */
unsigned char   LIS_fillbyte2;  /* filler byte */

unsigned short  LIS_AX;         /* @@V1 Environment selector */
unsigned short  LIS_BX;         /* @@V1 Offset of command line start */
unsigned short  LIS_CX;         /* @@V1 Length of Data Segment */
unsigned short  LIS_DX;         /* @@V1 STACKSIZE from the .EXE file */
unsigned short  LIS_SI;         /* @@V1 HEAPSIZE  from the .EXE file */
unsigned short  LIS_DI;         /* @@V1 Module handle of the application */
unsigned short  LIS_DS;         /* @@V1 Data Segment Handle of application */

unsigned short  LIS_PackSel;    /* First tiled selector in this EXE */
unsigned short  LIS_PackShrSel; /* First selector above shared arena */
unsigned short  LIS_PackPckSel; /* First selector above packed arena */
};

#define LIS_RealMode    LIS_ProcType
#define LIS_LEN         sizeof(struct InfoSegLDT)


/* XLATOFF */
/*
 *      The following definitions exist purely for compatibility
 *      with earlier versions of this file.  If new fields are
 *      added to the Local Infoseg, there is no need to add
 *      defines for them to this section.
 */

#define CurProcID       LIS_CurProcID
#define ParProcID       LIS_ParProcID
#define CurThrdPri      LIS_CurThrdPri
#define CurThrdID       LIS_CurThrdID
#define ProcStatus      LIS_ProcStatus
#define fillbyte1       LIS_fillbyte1
#define Fgnd            LIS_Fgnd
#define ProcType        LIS_ProcType
#define fillbyte2       LIS_fillbyte2
/* XLATON */




/*
 *      Process Type codes
 *
 *      These are the definitons for the codes stored
 *      in the LIS_ProcType field in the local infoseg.
 */

#define         LIS_PT_FULLSCRN 0       /* Full screen app. */
#define         LIS_PT_REALMODE 1       /* Real mode process */
#define         LIS_PT_VIOWIN   2       /* VIO windowable app. */
#define         LIS_PT_PRESMGR  3       /* Presentation Manager app. */
#define         LIS_PT_DETACHED 4       /* Detached app. */


/*
 *
 *      Process Status Bit Definitions
 *
 */

#define         LIS_PS_EXITLIST 0x01    /* In exitlist handler */


/*
 *      Flags equates for the Global Info Segment
 *      SIS_SysLog  WORD in Global Info Segment
 *
 *        xxxx xxxx xxxx xxx0         Error Logging Disabled
 *        xxxx xxxx xxxx xxx1         Error Logging Enabled
 *
 *        xxxx xxxx xxxx xx0x         Error Logging not available
 *        xxxx xxxx xxxx xx1x         Error Logging available
 */

#define LF_LOGENABLE    0x0001          /* Logging enabled */
#define LF_LOGAVAILABLE 0x0002          /* Logging available */


/* info on global and local infoseg copies */
struct infoseg_s {
    void           *is_va;              /* address in system arena */
    unsigned short  is_hob;             /* shared arena object handle */
                                        /*  BUGBUG - should be VMHOB is_hob */
    unsigned short  is_sel;             /* system arena selector */
    void           *is_ppf;             /* pointer to frame's pf structure */
};
