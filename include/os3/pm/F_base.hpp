#ifndef _F_BASE_HPP_
#define _F_BASE_HPP_
/*
 $Id: F_base.hpp,v 1.2 2003/07/14 21:21:18 evgen2 Exp $
*/
/* F_OS2.hpp */
/* OS2-specific includes and calls */
/* ver 0.01 16.08.2002 */

#ifndef FREEPM_BASE
  #define FREE_BASE

#include "F_def.hpp"

/* defines for errors */
#ifndef F_INCL_ERRORS
    #define F_INCL_ERRORS
#endif
#ifdef F_INCL_ERRORS
    #define INCL_ERRORS
#endif
#include <bseerr.h>

#ifdef __cplusplus
      extern "C" {
#endif

#ifdef F_INCL_DOS
   #define F_INCL_DOSPROCESS
   #define F_INCL_DOSFILEMGR
   #define F_INCL_DOSMEMMGR
   #define F_INCL_DOSSEMAPHORES
   #define F_INCL_DOSDATETIME
   #define F_INCL_DOSMODULEMGR
   #define F_INCL_DOSRESOURCES
   #define F_INCL_DOSNLS
   #define F_INCL_DOSEXCEPTIONS
   #define F_INCL_DOSMISC
   #define F_INCL_DOSMONITORS
   #define F_INCL_DOSQUEUES
   #define F_INCL_DOSSESMGR
   #define F_INCL_DOSDEVICES
   #define F_INCL_DOSNMPIPES
   #define F_INCL_DOSPROFILE
   #define F_INCL_DOSMVDM
#endif /* F_INCL_DOS */



#define APIENTRY    _System
typedef unsigned long int APIRET;

/*** Common DOS types */
/* HMODULE used in resources, so typedef for HMODULE is moved to basedef */
typedef LHANDLE HMODULE;        /* hmod */
typedef LHANDLE PID;            /* pid  */
typedef LHANDLE TID;            /* tid  */
typedef LHANDLE HFILE;          /* hf   */
typedef HFILE   *PHFILE;
typedef HMODULE *PHMODULE;
typedef PID *PPID;
typedef TID *PTID;
typedef void **PPVOID;
//typedef VOID APIENTRY (*PFNTHREAD) (ULONG);
//typedef VOID APIENTRY (FNTHREAD) (ULONG);
//typedef FNTHREAD *PFNTHREAD;
#define PAG_COMMIT 0x4
#define PAG_READ   0x1
#define PAG_WRITE  0x2

#ifdef F_INCL_DOSPROCESS
 #include <bsetib.h> /* Does not exist in OW. */
/* #define INCL_DOSPROCESS
   #include <bsedos.h>
*/
   APIRET APIENTRY DosKillThread(TID tid);

   APIRET APIENTRY DosAllocThreadLocalMemory(ULONG cb, PULONG *p);

   APIRET APIENTRY DosFreeThreadLocalMemory(ULONG *p);
   
   APIRET APIENTRY DosAllocMem(PPVOID ppb, ULONG cb, ULONG flag);

   /*** General services */
   APIRET APIENTRY DosSleep(ULONG msec);

   APIRET APIENTRY DosBeep(ULONG freq,
                           ULONG dur);

   APIRET APIENTRY DosGetInfoBlocks(PTIB *pptib,
                                    PPIB *pppib);
  
   //APIRET APIENTRY DosEnterCritSec(VOID);
   //APIRET APIENTRY DosExitCritSec(VOID); 
   //APIRET APIENTRY DosCreateThread(PTID, PFNTHREAD, ULONG, ULONG, ULONG);
   
/**** FreePM extension/utilities **********************/
/*
 The following process' type codes are available:

     0         Full screen protect-mode session
     1         Requires real mode. Dos emulation.
     2         VIO windowable protect-mode session
     3         Presentation Manager protect-mode session
     4         Detached protect-mode process.
*/
int QueryProcessType(void);
int QueryThreadOrdinal(int &tid);


#endif
     /* F_INCL_DOSPROCESS */

   APIRET APIENTRY  DosSetPriority(ULONG scope,
                                   ULONG ulClass,
                                   LONG  delta,
                                   ULONG PorTid);

   /* Priority scopes */

   #define PRTYS_PROCESS      0
   #define PRTYS_PROCESSTREE  1
   #define PRTYS_THREAD       2

   /* Priority classes */

   #define PRTYC_NOCHANGE     0
   #define PRTYC_IDLETIME     1
   #define PRTYC_REGULAR      2
   #define PRTYC_TIMECRITICAL 3
   #define PRTYC_FOREGROUNDSERVER 4

   /* Priority deltas */

   #define PRTYD_MINIMUM     -31
   #define PRTYD_MAXIMUM      31

/* from bsedos.h */
   /*************************************************************************\
   * CCHMAXPATH is the maximum fully qualified path name length including  *
   * the drive letter, colon, backslashes and terminating NULL.            *
   \*************************************************************************/
   #define CCHMAXPATH         260

   /*************************************************************************\
   * CCHMAXPATHCOMP is the maximum individual path component name length   *
   * including a terminating NULL.                                         *
   \*************************************************************************/
   #define CCHMAXPATHCOMP     256

/* DosOpen, DosCerateNpipe */
   /* DosOpen() open flags */
   #define FILE_OPEN       0x0001
   #define FILE_TRUNCATE   0x0002
   #define FILE_CREATE     0x0010

   /*     this nibble applies if file already exists                xxxx */
   #define OPEN_ACTION_FAIL_IF_EXISTS     0x0000  /* ---- ---- ---- 0000 */
   #define OPEN_ACTION_OPEN_IF_EXISTS     0x0001  /* ---- ---- ---- 0001 */
   #define OPEN_ACTION_REPLACE_IF_EXISTS  0x0002  /* ---- ---- ---- 0010 */

   /*     this nibble applies if file does not exist           xxxx      */
   #define OPEN_ACTION_FAIL_IF_NEW        0x0000  /* ---- ---- 0000 ---- */
   #define OPEN_ACTION_CREATE_IF_NEW      0x0010  /* ---- ---- 0001 ---- */

   /* DosOpen/DosSetFHandState mode flags */
   #define OPEN_ACCESS_READONLY           0x0000  /* ---- ---- ---- -000 */
   #define OPEN_ACCESS_WRITEONLY          0x0001  /* ---- ---- ---- -001 */
   #define OPEN_ACCESS_READWRITE          0x0002  /* ---- ---- ---- -010 */
   #define OPEN_SHARE_DENYREADWRITE       0x0010  /* ---- ---- -001 ---- */
   #define OPEN_SHARE_DENYWRITE           0x0020  /* ---- ---- -010 ---- */
   #define OPEN_SHARE_DENYREAD            0x0030  /* ---- ---- -011 ---- */
   #define OPEN_SHARE_DENYNONE            0x0040  /* ---- ---- -100 ---- */
   #define OPEN_FLAGS_NOINHERIT           0x0080  /* ---- ---- 1--- ---- */
   #define OPEN_FLAGS_NO_LOCALITY         0x0000  /* ---- -000 ---- ---- */
   #define OPEN_FLAGS_SEQUENTIAL          0x0100  /* ---- -001 ---- ---- */
   #define OPEN_FLAGS_RANDOM              0x0200  /* ---- -010 ---- ---- */
   #define OPEN_FLAGS_RANDOMSEQUENTIAL    0x0300  /* ---- -011 ---- ---- */
   #define OPEN_FLAGS_NO_CACHE            0x1000  /* ---1 ---- ---- ---- */
   #define OPEN_FLAGS_FAIL_ON_ERROR       0x2000  /* --1- ---- ---- ---- */
   #define OPEN_FLAGS_WRITE_THROUGH       0x4000  /* -1-- ---- ---- ---- */
   #define OPEN_FLAGS_DASD                0x8000  /* 1--- ---- ---- ---- */
   #define OPEN_FLAGS_NONSPOOLED          0x00040000
   #define OPEN_SHARE_DENYLEGACY       0x10000000   /* 2GB */
   #define OPEN_FLAGS_PROTECTED_HANDLE 0x40000000

      /*NOINC*/
      #pragma pack(1)
      /*INC*/


      typedef struct _FEA2         /* fea2 */
      {
         ULONG   oNextEntryOffset;    /* new field */
         BYTE    fEA;
         BYTE    cbName;
         USHORT  cbValue;
         CHAR    szName[1];           /* new field */
      } FEA2;
      typedef FEA2 *PFEA2;


      typedef struct _FEA2LIST     /* fea2l */
      {
         ULONG   cbList;
         FEA2    list[1];
      } FEA2LIST;
      typedef FEA2LIST *PFEA2LIST;



      typedef struct _GEA2          /* gea2 */
      {
         ULONG   oNextEntryOffset;     /* new field */
         BYTE    cbName;
         CHAR    szName[1];            /* new field */
      } GEA2;
      typedef GEA2 *PGEA2;

      typedef struct _GEA2LIST      /* gea2l */
      {
         ULONG   cbList;
         GEA2    list[1];
      } GEA2LIST;
      typedef GEA2LIST *PGEA2LIST;

      typedef struct _EAOP2         /* eaop2 */
      {
         PGEA2LIST   fpGEA2List;       /* GEA set */
         PFEA2LIST   fpFEA2List;       /* FEA set */
         ULONG       oError;           /* offset of FEA error */
      } EAOP2;
      typedef EAOP2 *PEAOP2;

   /*NOINC*/
   #pragma pack()
   /*INC*/

   APIRET APIENTRY  DosOpen(PCSZ     pszFileName,
                            PHFILE pHf,
                            PULONG pulAction,
                            ULONG  cbFile,
                            ULONG  ulAttribute,
                            ULONG  fsOpenFlags,
                            ULONG  fsOpenMode,
                            PEAOP2 peaop2);


   APIRET APIENTRY  DosClose(HFILE hFile);
   APIRET APIENTRY  DosRead(HFILE hFile,
                            PVOID pBuffer,
                            ULONG cbRead,
                            PULONG pcbActual);
   APIRET APIENTRY  DosWrite(HFILE hFile,
                             PVOID pBuffer,
                             ULONG cbWrite,
                             PULONG pcbActual);


   APIRET APIENTRY  DosSetRelMaxFH(PLONG pcbReqCount,
                                   PULONG pcbCurMaxFH);

#ifdef F_INCL_DOSMISC

   /* definitions for DosSearchPath control word */
   #define DSP_IMPLIEDCUR          1 /* current dir will be searched first */
   #define DSP_PATHREF             2 /* from env.variable */
   #define DSP_IGNORENETERR        4 /* ignore net errs & continue search */

   /* indices for DosQuerySysInfo */
   #define QSV_MAX_PATH_LENGTH        1
   #define Q_MAX_PATH_LENGTH          QSV_MAX_PATH_LENGTH
   #define QSV_MAX_TEXT_SESSIONS      2
   #define QSV_MAX_PM_SESSIONS        3
   #define QSV_MAX_VDM_SESSIONS       4
   #define QSV_BOOT_DRIVE             5 /* 1=A, 2=B, etc.                     */
   #define QSV_DYN_PRI_VARIATION      6 /* 0=Absolute, 1=Dynamic              */
   #define QSV_MAX_WAIT               7 /* seconds                            */
   #define QSV_MIN_SLICE              8 /* milli seconds                      */
   #define QSV_MAX_SLICE              9 /* milli seconds                      */
   #define QSV_PAGE_SIZE             10
   #define QSV_VERSION_MAJOR         11
   #define QSV_VERSION_MINOR         12
   #define QSV_VERSION_REVISION      13 /* Revision letter                    */
   #define QSV_MS_COUNT              14 /* Free running millisecond counter   */
   #define QSV_TIME_LOW              15 /* Low dword of time in seconds       */
   #define QSV_TIME_HIGH             16 /* High dword of time in seconds      */
   #define QSV_TOTPHYSMEM            17 /* Physical memory on system          */
   #define QSV_TOTRESMEM             18 /* Resident memory on system          */
   #define QSV_TOTAVAILMEM           19 /* Available memory for all processes */
   #define QSV_MAXPRMEM              20 /* Avail private mem for calling proc */
   #define QSV_MAXSHMEM              21 /* Avail shared mem for calling proc  */
   #define QSV_TIMER_INTERVAL        22 /* Timer interval in tenths of ms     */
   #define QSV_MAX_COMP_LENGTH       23 /* max len of one component in a name */
   #define QSV_FOREGROUND_FS_SESSION 24 /* Session ID of current fgnd FS session*/
   #define QSV_FOREGROUND_PROCESS    25 /* Process ID of current fgnd process */
   #define QSV_NUMPROCESSORS         26
   #define QSV_MAXHPRMEM             27
   #define QSV_MAXHSHMEM             28
   #define QSV_MAXPROCESSES          29
   #define QSV_VIRTUALADDRESSLIMIT   30
   #define QSV_INT10ENABLED          31
   #define QSV_MAX                   QSV_INT10ENABLED

   APIRET APIENTRY  DosQuerySysInfo(ULONG iStart,
                                    ULONG iLast,
                                    PVOID pBuf,
                                    ULONG cbBuf);

   APIRET APIENTRY  DosSearchPath(ULONG flag,
                                  PSZ pszPathOrName,
                                  PSZ pszFilename,
                                  PBYTE pBuf,
                                  ULONG cbBuf);

#endif /* F_INCL_DOSMISC */

   /*** Data structures used with named pipes ***/
#ifdef F_INCL_DOSNMPIPES

   typedef LHANDLE HPIPE;     /* hp */
   typedef HPIPE   *PHPIPE;

   /* DosCreateNPipe open modes */

   #define NP_ACCESS_INBOUND       0x0000
   #define NP_ACCESS_OUTBOUND      0x0001
   #define NP_ACCESS_DUPLEX        0x0002
   #define NP_INHERIT              0x0000
   #define NP_NOINHERIT            0x0080
   #define NP_WRITEBEHIND          0x0000
   #define NP_NOWRITEBEHIND        0x4000

   /* DosCreateNPipe and DosQueryNPHState state */

   #define NP_READMODE_BYTE        0x0000
   #define NP_READMODE_MESSAGE     0x0100
   #define NP_TYPE_BYTE            0x0000
   #define NP_TYPE_MESSAGE         0x0400
   #define NP_END_CLIENT           0x0000
   #define NP_END_SERVER           0x4000
   #define NP_WAIT                 0x0000
   #define NP_NOWAIT               0x8000
   #define NP_UNLIMITED_INSTANCES  0x00FF


   APIRET APIENTRY  DosCreateNPipe(PSZ pszName,
                                   PHPIPE pHpipe,
                                   ULONG openmode,
                                   ULONG pipemode,
                                   ULONG cbInbuf,
                                   ULONG cbOutbuf,
                                   ULONG msec);


   APIRET APIENTRY  DosCallNPipe(PSZ pszName,
                                 PVOID pInbuf,
                                 ULONG cbIn,
                                 PVOID pOutbuf,
                                 ULONG cbOut,
                                 PULONG pcbActual,
                                 ULONG msec);

   APIRET APIENTRY  DosConnectNPipe(HPIPE hpipe);

   APIRET APIENTRY  DosDisConnectNPipe(HPIPE hpipe);

   APIRET APIENTRY  DosQueryNPHState(HPIPE hpipe,
                                     PULONG pState);

   APIRET APIENTRY  DosQueryNPipeInfo(HPIPE hpipe,
                                      ULONG infolevel,
                                      PVOID pBuf,
                                      ULONG cbBuf);

   /* defined bits in pipe mode */
   #define NP_NBLK                    0x8000 /* non-blocking read/write */
   #define NP_SERVER                  0x4000 /* set if server end       */
   #define NP_WMESG                   0x0400 /* write messages          */
   #define NP_RMESG                   0x0100 /* read as messages        */
   #define NP_ICOUNT                  0x00FF /* instance count field    */

#endif
    /* F_INCL_DOSNMPIPES */

#ifdef F_INCL_DOSSEMAPHORES
   typedef  ULONG    HMTX;            /* hmtx */
   typedef  HMTX     *PHMTX;
   typedef  ULONG    HMUX;            /* hmux */
   typedef  HMUX     *PHMUX;

   APIRET APIENTRY  DosCreateMutexSem (PCSZ  pszName,
                                       PHMTX phmtx,
                                       ULONG flAttr,
                                       BOOL32 fState);
   APIRET APIENTRY  DosOpenMutexSem (PCSZ  pszName,
                                     PHMTX phmtx);
   APIRET APIENTRY  DosQueryMutexSem (HMTX hmtx,
                                      PID *ppid,
                                      TID *ptid,
                                      PULONG pulCount);
   APIRET APIENTRY  DosRequestMutexSem (HMTX hmtx,
                                        ULONG ulTimeout);
   APIRET APIENTRY  DosReleaseMutexSem (HMTX hmtx);
   APIRET APIENTRY  DosCloseMutexSem (HMTX hmtx);

#endif
   /* F_INCL_DOSSEMAPHORES */

#ifdef F_INCL_DOSDATETIME

/* Date and time data structure. */
 typedef struct _DATETIME {
   UCHAR      hours;       /*  Current hour, using values 0 through 23. */
   UCHAR      minutes;     /*  Current minute, using values 0 through 59. */
   UCHAR      seconds;     /*  Current second, using values 0 through 59. */
   UCHAR      hundredths;  /*  Current hundredths of a second, using values 0 through 99. */
   UCHAR      day;         /*  Current day of the month, using values 1 through 31. */
   UCHAR      month;       /*  Current month of the year, using values 1 through 12. */
   USHORT     year;        /*  Current year. */
   SHORT      timezone;    /*  The difference in minutes between the current time zone and Greenwich Mean Time (GMT). */
   UCHAR      weekday;     /*  Current day of the week, using values 0 through 6. */
 } DATETIME;

 typedef DATETIME *PDATETIME;

   APIRET APIENTRY   DosGetDateTime(PDATETIME pdt);
   APIRET APIENTRY   DosSetDateTime(PDATETIME pdt);

#endif
   /* F_INCL_DOSDATETIME */

#ifdef F_INCL_DOSMODULEMGR

   APIRET APIENTRY  DosLoadModule(PCSZ  pszName,
                                  ULONG cbName,
                                  PCSZ  pszModname,
                                  PHMODULE phmod);

   APIRET APIENTRY  DosFreeModule(HMODULE hmod);

   APIRET APIENTRY  DosQueryProcAddr(HMODULE hmod,
                                     ULONG ordinal,
                                     PCSZ  pszName,
                                     PFN* ppfn);

   APIRET APIENTRY  DosQueryModuleHandle(PSZ pszModname,
                                         PHMODULE phmod);

   APIRET APIENTRY  DosQueryModuleName(HMODULE hmod,
                                       ULONG cbName,
                                       PCHAR pch);
#endif 
  //F_INCL_DOSMODULEMGR

#ifdef __cplusplus
        }
#endif

#endif
   /* FREEPM_BASE */

#endif
  /* _F_BASE_HPP_ */

