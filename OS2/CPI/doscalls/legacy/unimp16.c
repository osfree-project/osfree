#include <strnlen.h>

#define INCL_MOU
#define INCL_KBD
#define INCL_VIO
#define INCL_PMAVIO
#define INCL_DOSNMPIPES
#define INCL_DOSPROCESS
//#define INCL_DOSMONITORS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSESMGR
#include <os2.h>

#ifndef HVPS
typedef USHORT HVPS;
typedef HVPS *PHVPS;
#endif

typedef struct _COUNTRYCODE {
    ULONG  country;
    ULONG  codepage;
} COUNTRYCODE, *PCOUNTRYCODE;

typedef struct _COUNTRYINFO {
    ULONG  country;
    ULONG  codepage;
    ULONG  fsDateFmt;
    CHAR   szCurrency[5];
    CHAR   szThousandsSeparator[2];
    CHAR   szDecimal[2];
    CHAR   szDateSeparator[2];
    CHAR   szTimeSeparator[2];
    UCHAR  fsCurrencyFmt;
    UCHAR  cDecimalPlace;
    UCHAR  fsTimeFmt;
    USHORT abReserved1[2];
    CHAR   szDataSeparator[2];
    USHORT abReserved2[5];
} COUNTRYINFO, *PCOUNTRYINFO;

typedef SHANDLE  HMONITOR;
typedef HMONITOR *PHMONITOR;

typedef int (APIENTRY16 _PFN16)();
typedef _PFN16  * _Seg16 PFN16;

typedef struct _PIDINFO
{
    PID pid;
    TID tid;
    PID ppid;
} PIDINFO, * _Seg16 PPIDINFO;

typedef BYTE * _Seg16 PCNPATH;

typedef BYTE * _Seg16 PCNINFO;

typedef LHANDLE HVDD, * _Seg16 PHVDD;

typedef ULONG HSPINLOCK, * _Seg16 PHSPINLOCK;

typedef struct {
   ULONG sl_vaddr; /* start of VA segment to profile */
   ULONG sl_size;  /* length of VA segment */
   ULONG sl_mode;  /* !=0 use PRF_VA* flags, */
                   /* =0, simple count */
} PRFSLOT;

typedef struct {
   PRFSLOT * _Seg16 cm_slots;      /* Virtual address slots */
   USHORT   cm_nslots;             /* # of VA slots < 256 (!) */
   USHORT   cm_flags;              /* command */
   ULONG    cm_bufsz;              /* reserve # of bytes for buffers */
                                   /* e.g. for hit buffer or detailed */
                                   /* counters */
   USHORT   cm_timval;             /* timer resolution */
                                   /* if 0, use default == 1000 */
   /* valid if PRF_FLAGBITS set */
   char * _Seg16  cm_flgbits;      /* vector of flag bits (?) */
   UCHAR    cm_nflgs;              /* # of flag bits >= 2 if present */
} PRFCMD; /* 19 bytes */

      #define PRF_RET_GLOBAL   0 /* return global data */
                                 /* set us_thrdno for specific thread */
                                 /* us_buf = struct PRFRET0 */
      #define PRF_RET_VASLOTS  1 /* return VA slot data (PRFRET1) */
      #define PRF_RET_VAHITS   2 /* return hit table (PRFRET2) */
      #define PRF_RET_VADETAIL 3 /* return detailed counters (PRFRET3) */
                                 /* specify us_vaddr */
typedef struct {
   UCHAR us_cmd;                /* command */
   USHORT us_thrdno;            /* thread requested for cmd=0 */
   ULONG us_vaddr;              /* VA for cmd=3*/
   ULONG us_bufsz;              /* length of return buffer */
   VOID *us_buf;                /* return buffer */
} PRFRET; /* 15 bytes */

typedef struct {
   USHORT r0_flags;             /* profile flags */
                                 /* see PRF_* defines */
   USHORT r0_shift;             /* shift factor */
                                 /* 2^N = length of a segment for */
                                 /* detailed counters */
   ULONG  r0_idle;              /* count if process is idle */
   ULONG  r0_vm86;              /* count if process is in VM mode */
   ULONG  r0_kernel;            /* count if process is in kernel */
   ULONG  r0_shrmem;            /* count if process is in shr mem */
   ULONG  r0_unknown;           /* count if process is elsewhere */
   ULONG  r0_nhitbufs;          /* # of dwords in hitbufs */
   ULONG  r0_hitbufcnt;         /* # of entries in hit table */
   ULONG  r0_reserved1;         /* internally used */
   ULONG  r0_reserved2;         /* internally used */
   USHORT r0_timval;            /* timer resolution */
   UCHAR  r0_errcnt;            /* error count */
   USHORT r0_nstruc1;           /* # of add structures 1 (?) */
   USHORT r0_nstruc2;           /* # of add structures 2 (?) */
} PRFRET0;

typedef struct {
   ULONG va_vaddr;              /* virtual address of segment */
   ULONG va_size;               /* length of segment */
   ULONG va_flags;              /* == 8, va_cnt is valid */
   ULONG va_reserved;           /* internally used */
   ULONG va_cnt;                /* profile count */
} PRFVA;

typedef struct {
   UCHAR r1_nslots;             /* # of slots (bug: prevents */
                                 /* correct # if #slots >255) */
   PRFVA r1_slots[1];           /* slots */
} PRFRET1;

typedef struct {
   ULONG r2_nhits;              /* # of entries in table */
   ULONG r2_hits[1];            /* hit table */
} PRFRET2;

typedef struct {
   ULONG r3_size;               /* size of segment */
   ULONG r3_ncnts;              /* # of entries in table */
   ULONG r3_cnts[1];            /* counters */
} PRFRET3;

typedef LHANDLE HQUEUE, * _Seg16 PHQUEUE;

APIRET unimplemented(char *func);


// fix prototype !!!
USHORT APIENTRY16 DOS16ALLOCPROTSEG(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!!
USHORT APIENTRY16 DOS16ALLOCSHRPROTSEG(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!!
USHORT APIENTRY16 DOS16ALLOCPROTHUGE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 DOS16CASEMAP(USHORT Length, COUNTRYCODE * _Seg16 Country, CHAR * _Seg16 BinaryString)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 DOS16GETCOLLATE(USHORT Length, COUNTRYCODE * _Seg16 Country,
                                  CHAR * _Seg16 MemoryBuffer, USHORT * _Seg16 DataLength)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 DOS16GETCTRYINFO(USHORT Length, COUNTRYCODE * _Seg16 Country,
                                   COUNTRYINFO * _Seg16 MemoryBuffer, USHORT * _Seg16 DataLength)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 DOS16GETDBCSEV(USHORT Length, COUNTRYCODE * _Seg16 Country, CHAR * _Seg16 MemoryBuffer)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16 DOS16DYNAMICTRACE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 DOS16MAKENMPIPE(CHAR * _Seg16 PipeName, HPIPE * _Seg16 PipeHandle, USHORT OpenMode,
                                  USHORT PipeMode, USHORT OutBufSize, USHORT InBufSize, ULONG TimeOut)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16  DOS16QNMPIPEINFO(HPIPE Handle, USHORT InfoLevel, BYTE * _Seg16 InfoBuf, USHORT InfoBufSize)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16  DOS16CONNECTNMPIPE(HPIPE handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16  DOS16DISCONNECTNMPIPE(HPIPE handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16  DOS16QNMPHANDSTATE(HPIPE Handle, USHORT * _Seg16 PipeHandleState)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16  DOS16SETNMPHANDSTATE(HPIPE Handle, USHORT PipeHandleState)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 DOS16PEEKNMPIPE(HPIPE Handle, BYTE * _Seg16 Buffer, USHORT BufferLen,
                                  USHORT * _Seg16 BytesRead, USHORT * _Seg16 BytesAvail,
                                  USHORT * _Seg16 PipeState)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16  DOS16WAITNMPIPE(CHAR * _Seg16 FileName, ULONG TimeOut)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16  DOS16CALLNMPIPE(CHAR * _Seg16 FileName, BYTE * _Seg16 InBuffer, USHORT InBufferLen,
                                       BYTE * _Seg16 OutBuffer, USHORT OutBufferLen, 
                                       USHORT * _Seg16 BytesOut, ULONG TimeOut)
{
  return unimplemented(__FUNCTION__);
}


// undoc (???)
USHORT APIENTRY16  DOS16RAWREADNMPIPE(HPIPE handle, USHORT cbBuf, BYTE * _Seg16 pBuf)
{
  return unimplemented(__FUNCTION__);
}


// undoc (???)
USHORT APIENTRY16  DOS16RAWWRITENMPIPE(HPIPE handle, USHORT cbBuf, BYTE * _Seg16 pBuf)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16  DOS16SETNMPIPESEM(HPIPE Handle, HSEM SemHandle, USHORT KeyHandle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16  DOS16QNMPIPESEMSTATE(HSEM SemHandle, BYTE * _Seg16 InfoBuf, USHORT InfoBufLen)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16  DOSICREATETHREAD(void)
{
  return unimplemented(__FUNCTION__);
}




// fix prototype !!! (internal)
APIRET APIENTRY16   DOSIEXECPGM(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16EXITLIST(USHORT FcnCode_Order, FNEXITLIST * _Seg16 RtnAddress)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOSGETINFOSEG(USHORT * _Seg16 pGlobalSeg,
                                    USHORT * _Seg16 pLocalSeg)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16GETPRTY(USHORT Scope, USHORT * _Seg16 Priority, USHORT id)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16KILLPROCESS(USHORT ActionCode, PID pid)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16SETPRTY(USHORT scope, USHORT PriorityClass,
                                 SHORT PriorityDelta, USHORT id)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16PTRACE(BYTE * _Seg16 PtraceB)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16HOLDSIGNAL(USHORT ActionCode)
{
  return unimplemented(__FUNCTION__);
}


typedef void _Far16 *PFNSIGHANDLER;


USHORT APIENTRY16   DOS16SETSIGHANDLER(PFNSIGHANDLER routine, PFNSIGHANDLER * _Seg16 prevAddr,
                                       PUSHORT * _Seg16 prevAction, USHORT Action, USHORT sigNumber)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16FLAGPROCESS(PID pid, USHORT ActionCode, USHORT FlagNum, USHORT FlagArg)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16MAKEPIPE(HFILE * _Seg16 ReadHandle, HFILE * _Seg16 WriteHandle,
                                  USHORT PipeSize)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16   DOSISYSSEMCLEAR(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16   DOSISEMREQUEST(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16   DOSISYSSEMSET(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16   DOSISEMWAIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16MUXSEMWAIT(USHORT * _Seg16 IndexNbr, void * _Seg16 ListAddr,
                                    LONG timeout)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16CLOSESEM(HSEM sem)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16CREATESEM(USHORT NoExclusive, HSEM * _Seg16 sem,
                                   char * _Seg16 SemName)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16OPENSEM(HSEM * _Seg16 sem, char * _Seg16 SemName)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16RESUMETHREAD(TID tid)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16SUSPENDTHREAD(TID tid)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16SETDATETIME(DATETIME * _Seg16 pdatetime)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16TIMERASYNC(ULONG interval, HSEM sem, HTIMER * _Seg16 timer)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16TIMERSTART(ULONG interval, HSEM sem, HTIMER * _Seg16 timer)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16TIMERSTOP(HTIMER timer)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16SLEEP(ULONG time)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16GETDATETIME(DATETIME * _Seg16 pdatetime)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16ALLOCSEG(USHORT size, SEL * _Seg16 psel,
                                  USHORT flags)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16ALLOCSHRSEG(USHORT size, char * _Seg16 name,
                                     SEL * _Seg16 psel)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16GETSHRSEG(char * _Seg16 name, SEL * _Seg16 psel)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16GIVESEG(SEL CallerSegSel, PID pid, SEL * _Seg16 RecepientSegSel)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16REALLOCSEG(USHORT size, SEL sel)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16FREESEG(SEL sel)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16ALLOCHUGE(USHORT numSeg, USHORT size, SEL * _Seg16 psel,
                                   USHORT MaxNumSeg, USHORT flags)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16GETHUGESHIFT(USHORT * _Seg16 ShiftCount)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16REALLOCHUGE(USHORT NumSeg, USHORT size, SEL sel)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16CREATECSALIAS(SEL DataSelector, SEL * _Seg16 CodeSelector)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOSLOADMODULE(char * _Seg16 pszNameBuf, USHORT cbBufLen,
                                    char * _Seg16 pszModName, HMODULE * _Seg16 phmod)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOSGETPROCADDR(HMODULE hmod, char * _Seg16 pszProcName,
                                     PFN16 ppfn)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOSFREEMODULE(HMODULE hmod)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16GETMODHANDLE(char * _Seg16 pszName, HMODULE * _Seg16 phmod)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16GETMODNAME(HMODULE hmod, USHORT cbBuf, char * _Seg16 szBuf)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16BEEP(USHORT freq, USHORT duration)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16CLIACCESS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16DEVCONFIG(void * _Seg16 DeviceInfo, USHORT item, USHORT parm)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOSDEVIOCTL(void * _Seg16 data, void * _Seg16 parm,
                                  USHORT func, USHORT cat, HFILE hf)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16   DOS16SGSWITCH(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16   DOS16SGSWITCHME(void)
{
  return unimplemented(__FUNCTION__);
}






USHORT APIENTRY16   DOS16CHGFILEPTR(HFILE hf, LONG distance,
                                    USHORT MoveType, ULONG * _Seg16 NewPointer)
{
  return unimplemented(__FUNCTION__);
}








USHORT APIENTRY16   DOS16FILELOCKS(HFILE hf, LONG * _Seg16 UnLockRange, LONG * _Seg16 LockRange)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16FINDCLOSE(HDIR hdir)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16FINDFIRST(char * _Seg16 pszFilename, HDIR * _Seg16 phdir,
                                   USHORT attr, FILEFINDBUF * _Seg16 pFileFindBuf,
                                   USHORT cbBuf, USHORT * _Seg16 pcount, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16FINDNEXT(HDIR hdir, FILEFINDBUF * _Seg16 pFileFindBuf,
                                  USHORT cbBuf, USHORT * _Seg16 pcount)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY16   DOS16MOVE(char * _Seg16 old, char * _Seg16 new, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16NEWSIZE(HFILE hf, ULONG size)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16PORTACCESS(USHORT reserved, USHORT type,
                                    USHORT firstport ,USHORT lastport)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOSOPEN(char * _Seg16 pszFileName, HFILE * _Seg16 phf,
                              USHORT * _Seg16 action, ULONG cbSize, USHORT attr,
                              USHORT usOpenFlags, USHORT usOpenMode, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16QCURDIR(USHORT drivenum, char * _Seg16 pszPath, USHORT * _Seg16 cbPath)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16QCURDISK(USHORT * _Seg16 drivenum, ULONG * _Seg16 drivemap)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16QFHANDSTATE(HFILE hf, USHORT * _Seg16 state)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16QFILEINFO(HFILE hf, USHORT level,
                                    char * _Seg16 pBuf, USHORT cbBuf)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16QFILEMODE(char * _Seg16 pszFilename, USHORT * _Seg16 attr,
                                    ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16QFSINFO(USHORT drivenum, USHORT level, char * _Seg16 pBuf,
                                  USHORT cbBuf)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16QHANDTYPE(HFILE hf, USHORT * _Seg16 type, USHORT * _Seg16 flag)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16    DOSIREAD(void)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY16    DOS16SETFHANDSTATE(HFILE hf, USHORT state)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16SETFILEINFO(HFILE hf, USHORT level, char * _Seg16 pBuf,
                                      USHORT cbBuf)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16SETFILEMODE(char * _Seg16 pszFilename, USHORT attr,
                                      ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}



USHORT APIENTRY16    DOS16SETMAXFH(USHORT usNumHandles)
{
  return unimplemented(__FUNCTION__);
}




// fix prototype !!! (internal)
APIRET APIENTRY16    DOSIWRITE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16    DOS16SYSTEMSERVICE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16SETVEC(USHORT usVecNum, PFN16 routine, PFN16 prev)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16    DOS16SYSTRACE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16GETENV(SEL * _Seg16 pEnvSel, USHORT * _Seg16 off)
{
  return unimplemented(__FUNCTION__);
}




// fix prototype !!! (undoc)
USHORT APIENTRY16    DOS16QTRACEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16GETPID(PIDINFO * _Seg16 info)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16OPEN2(char * _Seg16 pszFilename, HFILE * _Seg16 hf,
                                USHORT * _Seg16 action, ULONG cbFile, USHORT attr,
                                USHORT usOpenFlags, ULONG ulOpenMode, EAOP2 * _Seg16 peaop2,
                                ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16    DOS16LIBINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16SETFSINFO(USHORT drivenum, USHORT level,
                                    char * _Seg16 pBuf, USHORT cbBuf)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16QPATHINFO(char * _Seg16 pszName, USHORT level,
                                    char * _Seg16 pBuf, USHORT cbBuf, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16DEVIOCTL2(void * _Seg16 data, USHORT datalen,
                                    void * _Seg16 parm, USHORT parmlen,
                                    USHORT func, USHORT cat, HFILE hf)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16    DOSICANONICALIZE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16    DOS16SETFGND(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16    DOS16SWAPTASKINIT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16    DOS16READPHYS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16SETPATHINFO(char * _Seg16 pszName, USHORT level,
                                      char * _Seg16 pBuf, USHORT cbBuf,
                                      USHORT flags, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16    DOS16SGSWITCHPROC2(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16    STRUCHECK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16    STRURESUPDATE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16    DOSISETRELMAXFH(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16    DOSIDEVIOCTL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
USHORT APIENTRY16    DOS16GETSTDA(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16ERROR(USHORT flag)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16GETSEG(SEL sel)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16LOCKSEG(SEL sel)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16UNLOCKSEG(SEL sel)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16    DOS16SGSWITCHPROC(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16    DOSIRAMSEMWAKE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16SIZESEG(SEL sel, ULONG * _Seg16 size)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16MEMAVAIL(ULONG * _Seg16 size)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16    DOSIRAMSEMREQUEST(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16PHYSICALDISK(USHORT func, char * _Seg16 data,
                                       USHORT datalen, char * _Seg16 parm,
                                       USHORT parmlen)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16GETCP(USHORT len, USHORT * _Seg16 CodePageList,
                                USHORT * _Seg16 datalen)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16    DOSISETCP(USHORT codepage, USHORT reserved)
{
  return unimplemented(__FUNCTION__);
}


// Global Infoseg Selector
SEL DOS16GLOBALSEG;
#pragma aux DOS16GLOBALSEG "*"


USHORT APIENTRY16    DOS16PROFILE(ULONG func, PID pid, PRFCMD * _Seg16 profcmd,
                                  PRFRET * _Seg16 profret)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16    DOS16SENDSIGNAL(PID pid, USHORT sigNum)
{
  return unimplemented(__FUNCTION__);
}


// Huge Shift value
USHORT DOS16HUGESHIFT;
#pragma aux DOS16HUGESHIFT "*"


// Huge Increment value
USHORT DOS16HUGEINCR;
#pragma aux DOS16HUGEINCR "*"


USHORT APIENTRY16     DOS16READ(HFILE hf, void * _Seg16 pBuf,
                                USHORT cbBuf, USHORT * _Seg16 pcbRead)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16WRITE(HFILE hf, void * _Seg16 pBuf,
                                 USHORT cbBif, USHORT * _Seg16 pcbWritten)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16ERRCLASS(USHORT code, USHORT * _Seg16 class,
                                    USHORT * _Seg16 action, USHORT * _Seg16 locus)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SEMREQUEST(HSEM sem, LONG timeout)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SEMCLEAR(HSEM sem)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SEMWAIT(HSEM sem, LONG timeout)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SEMSET(HSEM sem)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16   DOS16SEMSETWAIT(HSEM sem, LONG Timeout)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16EXECPGM(char * _Seg16 szObjName, SHORT cbObjBuf,
                                   USHORT execflags, char * _Seg16 argptr,
                                   char * _Seg16 envptr, RESULTCODES * _Seg16 res,
                                   char * _Seg16 pgmptr)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16CREATETHREAD(FNTHREAD * _Seg16 pfnthread, TID * _Seg16 ptid,
                                        void * _Seg16 stack)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SUBSET(SEL sel, USHORT flags, USHORT size)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SUBALLOC(SEL sel, USHORT * _Seg16 blkOffset,
                                    USHORT size)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SUBFREE(SEL sel, USHORT blkOffset, USHORT size)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16READASYNC(HFILE hf, ULONG * _Seg16 ramsem,
                                     USHORT * _Seg16 retcode, void * _Seg16 pBuf,
                                     USHORT cbBuf, USHORT * _Seg16 cbRead)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16WRITEASYNC(HFILE hf, ULONG * _Seg16 ramsem,
                                      USHORT * _Seg16 retcode, void * _Seg16 pBuf,
                                      USHORT cbBuf, USHORT * _Seg16 cbRead)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SEARCHPATH(USHORT control, char * _Seg16 pszPath,
                                      char * _Seg16 pszFilename, BYTE * _Seg16 pBuf,
                                      USHORT cbBuf)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SCANENV(char * _Seg16 pszEnvVar, char * _Seg16 * _Seg16 ppszResult)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SETCP(USHORT codepage, USHORT reserved)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16QPROCSTATUS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16GETRESOURCE(HMODULE hmod, USHORT typeId,
                                       USHORT nameId, SEL * _Seg16 psel)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16GETPPID(PID pid, PID * _Seg16 ppid)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16CALLBACK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSICALLBACK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16RETFORWARD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16R2STACKREALLOC(USHORT cbNewSize)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16FSRAMSEMREQUEST(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16FSRAMSEMCLEAR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16QAPPTYPE(char * _Seg16 pszFilename, USHORT * _Seg16 apptype)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SETPROCCP(USHORT cp, USHORT reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16QSYSINFO(USHORT index, void * _Seg16 pBuf, USHORT cbBuf)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSIMAKENMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSICALLNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSICONNECTNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSIDISCONNECTNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSIPEEKNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSIQNMPIPEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSIQNMPHANDSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSISETNMPHANDSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSITRANSACTNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSIWAITNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSISETNMPIPESEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSIQNMPIPESEMSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSIRAWREADNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16     DOSIRAWWRITENMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16FSATTACH(char * _Seg16 pszDeviceName, char * _Seg16 fsdName,
                                    void * _Seg16 pBuf, USHORT cbBufLen, USHORT opFlag,
                                    ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16QFSATTACH(char * _Seg16 pszDeviceName, USHORT ordinal,
                                     USHORT fsaInfoLevel, void * _Seg16 pBuf,
                                     USHORT * _Seg16 pcbBuf, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16FSCTL(void * _Seg16 pData, USHORT cbDataLenMax, USHORT * _Seg16 pcbDataLen,
                                 void * _Seg16 pParm, USHORT cbParmLenMax, USHORT * _Seg16 pcbParmLen,
                                 USHORT func, char * _Seg16 pszRouteName, HFILE hf, USHORT usRouteMethod,
                                 ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16FINDFIRST2(char * _Seg16 pszFilename, HDIR * _Seg16 phdir, USHORT attr,
                                      void * _Seg16 pBuf, USHORT cbBuf, USHORT * _Seg16 psearchcnt,
                                      USHORT level, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16MKDIR2(char * _Seg16 pszDir, EAOP2 * _Seg16 peaop2, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16FILEIO(HFILE hf, void * _Seg16 pCmdList, USHORT cbList,
                                  LONG * _Seg16 ErrorOffset)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16FINDNOTIFYCLOSE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16FINDNOTIFYFIRST(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16FINDNOTIFYNEXT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16SETTRACEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16EDITNAME(USHORT level, char * _Seg16 pszSrc,
                                    char * _Seg16 pszEditStr, void * _Seg16 pBuf,
                                    USHORT cbBuf)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16      DOS16LOGMODE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16      DOS16LOGENTRY(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16      DOS16GETLOGBUFFER(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16      DOS16LOGREGISTER(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16      DOS16LOGREAD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16      DOS16FINDFROMNAME(HDIR hDir, void * _Seg16 pfindbuf, USHORT cbBuf,
                                     USHORT * _Seg16 pcFileNames, USHORT ulPosition,
                                     char * _Seg16 pszFileSpec)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16      DOS16OPLOCKRELEASE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16      DOS16OPLOCKWAIT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16      DOSICOPY(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16      DOS16COPY(char * _Seg16 pszSrc, char * _Seg16 pszDst,
                                 USHORT opMode, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16      DOSIQAPPTYPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16      DOS16FORCEDELETE(char * _Seg16 pszFile)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16      DOS16ENUMATTRIBUTE(USHORT refType, void * _Seg16 fileRef,
                                          ULONG ulEntryNum, void * _Seg16 pBuf,
                                          ULONG cbBuf, ULONG * _Seg16 cEnum,
                                          ULONG level, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16      DOS16OPLOCKSHUTDOWN(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16      DOS16SHUTDOWN(ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY      DOSICACHEMODULE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16      DOS16GETRESOURCE2(HMODULE hmod, USHORT typeId, USHORT nameId,
                                         ULONG * _Seg16 resAddr)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16      DOS16FREERESOURCE(BYTE * _Seg16 resAddr)
{
  return unimplemented(__FUNCTION__);
}


// max Path len value
USHORT DOS16MAXPATHLEN;
#pragma aux DOS16MAXPATHLEN "*"


// max processor page size value
USHORT DOS16PAGESIZE;
#pragma aux DOS16PAGESIZE "*"


// LIS selector value
SEL DOS16LOCALINFO;
#pragma aux DOS16LOCALINFO "*"


// GIS selector value
SEL DOS16GLOBALINFO;
#pragma aux DOS16GLOBALINFO "*"


USHORT APIENTRY16     DOS16OPENVDD(char * _Seg16 pszVDD, HVDD * _Seg16 phvdd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16REQUESTVDD(HVDD hvdd, SGID sgid, USHORT cmd,
                                      USHORT cbInput, void * _Seg16 pInput,
                                      USHORT cbOutput, void * _Seg16 pOutput)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16CLOSEVDD(HVDD hvdd)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16QUERYDOSPROPERTY(SGID sgid, char * _Seg16 pszName,
                                            ULONG cb, char * _Seg16 pszBuf)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SETDOSPROPERTY(SGID sgid, char * _Seg16 pszName,
                                          ULONG cb, char * _Seg16 pszBuf)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16INSMESSAGE(char * _Seg16 * _Seg16 ivTable, USHORT ivCount,
                                      char * _Seg16 pszMsg, USHORT cbMsgLen, char * _Seg16 pData,
                                      USHORT cbDataLen, USHORT * _Seg16 cbMsgActual)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16IQUERYMESSAGECP(char * _Seg16 pb, USHORT cb,
                                        char * _Seg16 pszFile,
                                        PUSHORT cbBuf, void * _Seg16 msgSeg)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16TRANSACTNMPIPE(HPIPE hpipe, void * _Seg16 pInBuf, USHORT cbInBuf,
                                          void * _Seg16 pOutBuf, USHORT cbOutBuf, USHORT * _Seg16 pcbOut)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16TMRQUERYFREQ(ULONG * _Seg16 pulFreq)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16TMRQUERYTIME(QWORD * _Seg16 pqwTime)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16REGISTERPERFCTRS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16FLATTOSEL(ULONG addr)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16SELTOFLAT(ULONG addr)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16OPENCHANGENOTIFY(PCNPATH PathBuf,
                                            USHORT LogSize,
                                            HDIR * _Seg16 hdir,
                                            ULONG ulReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16RESETCHANGENOTIFY(PCNINFO LogBuf,
                                             USHORT BufferSize,
                                             USHORT * _Seg16 LogCount,
                                             HDIR hdir)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16CLOSECHANGENOTIFY(HDIR hdir)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16QUERYABIOSSUPPORT(USHORT reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16CREATESPINLOCK(PHSPINLOCK pHandle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16ACQUIRESPINLOCK(HSPINLOCK Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16RELEASESPINLOCK(HSPINLOCK Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16FREESPINLOCK(HSPINLOCK Handle)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16R3EXITADDR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16QUERYRESOURCESIZE(HMODULE hmod, USHORT idt,
                                             USHORT idn, USHORT * _Seg16 pulSize)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16INITIALIZEPORTHOLE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16QUERYHEADERINFO(HMODULE hmod, USHORT usIndex, void * _Seg16 pvBuffer,
                                           USHORT cbBuffer, USHORT usSubFunction)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16QUERYPROCTYPE(HMODULE hmod, USHORT ordinal,
                                         char * _Seg16 pszName, USHORT * _Seg16 pulProcType)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16PROTECTCLOSE(HFILE hFile,
                                        FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16     DOS16PROTECTFILEIO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16PROTECTFILELOCKS(HFILE hFile,
                                            FILELOCKL * _Seg16 pflUnlock,
                                            FILELOCKL * _Seg16 pflLock,
                                            ULONG timeout, USHORT flags,
                                            FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16PROTECTNEWSIZE(HFILE hFile,
                                          LONG cbSize,
                                          FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16PROTECTOPEN(char * _Seg16 pszFileName,
                                       HFILE * _Seg16 phf,
                                       ULONG * _Seg16 pulAction,
                                       LONG cbFile,
                                       USHORT usAttribute,
                                       USHORT fsOpenFlags,
                                       USHORT fsOpenMode,
                                       EAOP2 * _Seg16 peaop2,
                                       FHLOCK * _Seg16 pfhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16PROTECTQFHANDSTATE(HFILE hFile,
                                              USHORT * _Seg16 pMode,
                                              FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16PROTECTSETFHANDSTATE(HFILE hFile,
                                                USHORT mode,
                                                FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16     DOS16PROTECTQFILEINFO(HFILE hf,
                                            USHORT usInfoLevel,
                                            void * _Seg16 pInfo,
                                            USHORT cbInfoBuf,
                                            FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16      DOS16PROTECTSETFILEINFO(HFILE hf,
                                               USHORT usInfoLevel,
                                               void * _Seg16 pInfoBuf,
                                               USHORT cbInfoBuf,
                                               FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}



USHORT APIENTRY16       DOS16PROTECTCHGFILEPTR(HFILE hFile,
                                               LONG ib,
                                               USHORT method,
                                               LONG * _Seg16 ibActual,
                                               FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 DOS16PROTECTENUMATTRIBUTE(USHORT usRefType,
                                            void * _Seg16 pvFile,
                                            ULONG usEntry,
                                            void * _Seg16 pvBuf,
                                            ULONG cbBuf,
                                            ULONG * _Seg16 pusCount,
                                            ULONG ulInfoLevel,
                                            FHLOCK fhFileHandleLockID)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16LDRDIRTYWORKER(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16READQUEUE(HQUEUE hq, ULONG * _Seg16 req,
                                       USHORT * _Seg16 pDataLen, ULONG * _Seg16 pDataAddr,
                                       USHORT elCode, UCHAR NoWait, BYTE * _Seg16 elPriority,
                                       HSEM sem)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16PURGEQUEUE(HQUEUE hq)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16CLOSEQUEUE(HQUEUE hq)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16QUERYQUEUE(HQUEUE hq, USHORT * _Seg16 numelem)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16PEEKQUEUE(HQUEUE hq, ULONG * _Seg16 req, USHORT * _Seg16 datalen,
                                       ULONG * _Seg16 dataaddr, USHORT * _Seg16 elcode, UCHAR NoWait,
                                       BYTE * _Seg16 elPriority, ULONG sem)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16WRITEQUEUE(HQUEUE hq, USHORT req, USHORT datalen,
                                        BYTE * _Seg16 pDataBuf, UCHAR elPriority)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16OPENQUEUE(PID * _Seg16 ownerPid, HQUEUE * _Seg16 phq,
                                       char * _Seg16 pszQueName)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16CREATEQUEUE(HQUEUE * _Seg16 phq, USHORT quePrty,
                                         char * _Seg16 pszQueName)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMGETMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMFREEMEM(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMGETSGCB(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMINITSGCB(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMSGDOPOPUP(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMSWITCH(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMSERVEAPPREQ(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16GETTIMES(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMSETTITLE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SCRUNLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMDOAPPREQ(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16STOPSESSION(USHORT option, USHORT sessid,
                                         ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16SELECTSESSION(USHORT sessid, ULONG reserved)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SCRLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SAVREDRAWWAIT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SAVREDRAWUNDO(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMSGENDPOPUP(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16SETSESSION(USHORT sessid, STATUSDATA * _Seg16 statusdata)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SETMNLOCKTIME(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16MODEUNDO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16       DOS16STARTSESSION(STARTDATA * _Seg16 startdata, USHORT * _Seg16 sessid,
                                          USHORT * _Seg16 pid)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMGETSTATUS(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16MODEWAIT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMTERMINATE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMGETAPPREQ(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMINITIALIZE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMSTART(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMPARENTSWITCH(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMPAUSE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMHDEINIT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMPMPRESENT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMREGISTERDD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMNOTIFYDD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMNOTIFYDD2(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMOPENDD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16       DOS16SMSETSESSIONTYPE(void)
{
  return unimplemented(__FUNCTION__);
}



USHORT APIENTRY16 KBDREGISTER(const char * _Seg16 pszModName, const char * _Seg16 pszEntryPt,
                            const ULONG FunMask)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 KBDDEREGISTER(VOID)
{
  return unimplemented(__FUNCTION__);
}

USHORT APIENTRY16 KBDSYNCH(const USHORT fsWait)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 KBDSETFGND(VOID)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 KBDSETHWID(const KBDHWID * _Seg16 pkbdhwid, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}

// fix prototype !!! (undoc)
USHORT APIENTRY16       KBDINIT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16         KBDSHELLINIT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16         KBDSWITCHFGND(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16         KBDLOADINSTANCE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16         KBDFREE(void)
{
  return unimplemented(__FUNCTION__);
}




// fix prototype !!! (undoc)
USHORT APIENTRY16 BVSMAIN(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16 BVSREDRAWSIZE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16 BVSGETPTRDRAWNAME(void)
{
  return unimplemented(__FUNCTION__);
}



// fix prototype !!! (undoc)
USHORT APIENTRY16         DOS16SMSYSINIT(void)
{
  return unimplemented(__FUNCTION__);
}


// keyboard handle value
USHORT QhKeybdHandle;
#pragma aux QhKeybdHandle "*"


// mouse handle value
USHORT QhMouseHandle;
#pragma aux QhMouseHandle "*"


// SM Queue RAM Sem
HSEM SMQueueRamSem;
#pragma aux SMQueueRamSem "*"


// SM Array
USHORT SMArray[];
#pragma aux SMArray "*"


// SM PID Array
PID SMPIDArray[];
#pragma aux SMPIDArray "*"


// SM Initialized value
USHORT SMInitialized;
#pragma aux SMInitialized "*"


// SM Array size
USHORT SMArraySize;
#pragma aux SMArraySize "*"


// fix prototype !!! (undoc)
USHORT APIENTRY16         BVSGLOBAL(void)
{
  return unimplemented(__FUNCTION__);
}


// SMG Instance
USHORT SMGINSTANCE;
#pragma aux SMGINSTANCE "*"


// fix prototype !!! (undoc)
USHORT APIENTRY16         DOSPM16SEMRST(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16         DOS16FSRAMSEMREQUEST2(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16         DOS16FSRAMSEMCLEAR2(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         DOS16SETEXTLIBPATH(const char * _Seg16 pszExtLIBPATH, USHORT flags)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16         DOS16QUERYEXTLIBPATH(const char * _Seg16 pszExtLIBPATH, USHORT flags)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16         DOSPM16SETRST(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16         DOSPM16SEMCHK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16         THK16_UNITHUNK(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16         HT16_STARTUP(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (undoc)
USHORT APIENTRY16         HT16_CLEANUP(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16         DOSISETFILEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16         DOSISETPATHINFO(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16      DOSIFINDNEXT(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16      DOSISIGDISPATCH(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16      DOSIRAISEEXCEPTION(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16      DOSIQUERYFHSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16      DOSISETFHSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16      DOSIFINDFIRST(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16         DOSIPROTECTSETFILEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16         DOSIPROTECTREAD(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16         DOSIPROTECTWRITE(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16         DOSIOPENL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16         DOSIPROTECTOPENL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16         DOSISETFILESIZEL(void)
{
  return unimplemented(__FUNCTION__);
}


// fix prototype !!! (internal)
APIRET APIENTRY16         DOSIPROTECTSETFILESIZEL(void)
{
  return unimplemented(__FUNCTION__);
}

/* MouCalls */


USHORT APIENTRY16 MOUREGISTER(const char * _Seg16 pszModName, const char * _Seg16 pszEntryName,
                              const ULONG flFuns)
{
  return unimplemented(__FUNCTION__);
}


USHORT APIENTRY16 MOUDEREGISTER(VOID)
{
  return unimplemented(__FUNCTION__);
}

//USHORT APIENTRY16 MOUGETTHRESHOLD(THRESHOLD * _Seg16 pthreshold, const HMOU hmou)
USHORT APIENTRY16 MOUGETTHRESHOLD(void * _Seg16 pthreshold, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


//USHORT APIENTRY16 MOUSETTHRESHOLD(const THRESHOLD * _Seg16 pthreshold, const HMOU hmou)
USHORT APIENTRY16 MOUSETTHRESHOLD(void * _Seg16 pthreshold, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}
