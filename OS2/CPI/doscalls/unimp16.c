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

typedef struct _QMRESULT{
    USHORT seg;
    USHORT htme;
    char name[256];
} QMRESULT;

typedef QMRESULT *PQMRESULT;

typedef SHANDLE  HMONITOR;
typedef HMONITOR *PHMONITOR;

APIRET unimplemented(char *func);

APIRET APIENTRY      DosAllocProtSeg(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosAllocShrProtSeg(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosAllocProtHuge(void)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosCaseMap(USHORT Length, PCOUNTRYCODE Country, PCHAR BinaryString)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosGetCollate(USHORT Length, PCOUNTRYCODE Country,
                                   PCHAR MemoryBuffer, PUSHORT DataLength)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosGetCtryInfo(USHORT Length, PCOUNTRYCODE Country,
                                    PCOUNTRYINFO MemoryBuffer, PUSHORT DataLength)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosGetDBCSEv(USHORT Length, PCOUNTRYCODE Country, PCHAR MemoryBuffer)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosDynamicTrace(void)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosQueryModFromCS(SEL sel, PQMRESULT qmresult)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosMakeNmPipe(PSZ PipeName, PHPIPE PipeHandle, USHORT OpenMode,
                                   USHORT PipeMode, USHORT OutBufSize, USHORT InBufSize, ULONG TimeOut)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosQNmPipeInfo(HPIPE Handle, USHORT InfoLevel, PBYTE InfoBuf, USHORT InfoBufSize)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosConnectNmPipe(HPIPE handle)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosDisconnectNmPipe(HPIPE handle)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosQNmPHandState(HPIPE Handle, PUSHORT PipeHandleState)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosSetNmPHandState(HPIPE Handle, USHORT PipeHandleState)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosPeekNmPipe(HPIPE Handle, PBYTE Buffer, USHORT BufferLen,
                                   PUSHORT BytesRead, PUSHORT BytesAvail, PUSHORT PipeState)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosWaitNmPipe(PSZ FileName, ULONG TimeOut)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosCallNmPipe(PSZ FileName, PBYTE InBuffer, USHORT InBufferLen,
                                   PBYTE OutBuffer, USHORT OutBufferLen, 
                                   PUSHORT BytesOut, ULONG TimeOut)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosRawReadNmPipe(HPIPE handle, USHORT cbBuf, PBYTE pBuf) // undoc (???)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosRawWriteNmPipe(HPIPE handle, USHORT cbBuf, PBYTE pBuf) // undoc (???)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosSetNmPipeSem(HPIPE Handle, HSEM SemHandle, USHORT KeyHandle)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosQNmPipeSemState(HSEM SemHandle, PBYTE InfoBuf, USHORT InfoBufLen)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      Dos16MonOpen(PSZ Devname, PHMONITOR Handle)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      Dos16MonClose(HMONITOR handle)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      Dos16MonRead(PBYTE BufferI, UCHAR WaitFlag, PBYTE DataBuffer, PUSHORT Bytecnt)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      Dos16MonWrite(PBYTE BufferO, PBYTE DataBuffer, USHORT Bytecnt)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      Dos16MonReg(HMONITOR Handle, PBYTE BufferI, PBYTE BufferO,
                               USHORT Posflag, USHORT Index)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DosICreateThread(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CWAIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16ENTERCRITSEC(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIExecPgm(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16EXIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16EXITCRITSEC(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16EXITLIST(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETINFOSEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETPRTY(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16KILLPROCESS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETPRTY(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PTRACE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16HOLDSIGNAL(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETSIGHANDLER(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FLAGPROCESS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16MAKEPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISysSemClear(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISemRequest(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISysSemSet(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SEMSETWAIT(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISemWait(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16MUXSEMWAIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CLOSESEM(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CREATESEM(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16OPENSEM(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16RESUMETHREAD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SUSPENDTHREAD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETDATETIME(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16TIMERASYNC(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16TIMERSTART(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16TIMERSTOP(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SLEEP(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETDATETIME(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16ALLOCSEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16ALLOCSHRSEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETSHRSEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GIVESEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16REALLOCSEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FREESEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16ALLOCHUGE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETHUGESHIFT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16REALLOCHUGE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CREATECSALIAS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16LOADMODULE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETPROCADDR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FREEMODULE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETMODHANDLE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETMODNAME(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETMACHINEMODE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16BEEP(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CLIACCESS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16DEVCONFIG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16DEVIOCTL(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SGSWITCH(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SGSWITCHME(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16BUFRESET(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CHDIR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CHGFILEPTR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CLOSE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16DELETE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16DUPHANDLE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FILELOCKS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FINDCLOSE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FINDFIRST(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FINDNEXT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16MKDIR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16MOVE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16NEWSIZE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PORTACCESS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16OPEN(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QCURDIR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QCURDISK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QFHANDSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QFILEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QFILEMODE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QFSINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QHANDTYPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QVERIFY(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIRead(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16RMDIR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SELECTDISK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETFHANDSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETFILEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETFILEMODE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETMAXFH(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETVERIFY(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIWrite(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SYSTEMSERVICE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETVEC(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SYSTRACE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETENV(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETVERSION(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QTRACEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETPID(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16OPEN2(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16LIBINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETFSINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QPATHINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16DEVIOCTL2(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosICanonicalize(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETFGND(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SWAPTASKINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16READPHYS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETPATHINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SGSWITCHPROC2(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         STRUCHECK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         STRURESUPDATE(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISetRelMaxFH(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIDevIOCtl(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETSTDA(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16ERROR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETSEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16LOCKSEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16UNLOCKSEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SGSWITCHPROC(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIRamSemWake(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SIZESEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16MEMAVAIL(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosIRamSemRequest(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PHYSICALDISK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETCP(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DosISetCP(USHORT codepage, USHORT reserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GLOBALSEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROFILE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SENDSIGNAL(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16HUGESHIFT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16HUGEINCR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16READ(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16WRITE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16ERRCLASS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SEMREQUEST(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SEMCLEAR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SEMWAIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SEMSET(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16EXECPGM(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CREATETHREAD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SUBSET(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SUBALLOC(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SUBFREE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16READASYNC(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16WRITEASYNC(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SEARCHPATH(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SCANENV(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETCP(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QPROCSTATUS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETRESOURCE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETPPID(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CALLBACK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16RETFORWARD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16R2STACKREALLOC(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FSRAMSEMREQUEST(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FSRAMSEMCLEAR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QAPPTYPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETPROCCP(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16DYNAMICTRACE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QSYSINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FSATTACH(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QFSATTACH(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FSCTL(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FINDFIRST2(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16MKDIR2(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FILEIO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FINDNOTIFYCLOSE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FINDNOTIFYFIRST(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FINDNOTIFYNEXT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETTRACEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16EDITNAME(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16LOGMODE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16LOGENTRY(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETLOGBUFFER(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16LOGREGISTER(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16LOGREAD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FINDFROMNAME(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16OPLOCKRELEASE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16OPLOCKWAIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16COPY(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FORCEDELETE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16ENUMATTRIBUTE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16OPLOCKSHUTDOWN(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SHUTDOWN(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETRESOURCE2(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FREERESOURCE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16MAXPATHLEN(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PAGESIZE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16LOCALINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GLOBALINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16OPENVDD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16REQUESTVDD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CLOSEVDD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16ALLOCPROTSEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16ALLOCSHRPROTSEG(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16ALLOCPROTHUGE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QUERYDOSPROPERTY(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETDOSPROPERTY(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PUTMESSAGE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16TRUEGETMESSAGE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16INSMESSAGE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16IQUERYMESSAGECP(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CASEMAP(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETCOLLATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETCTRYINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETDBCSEV(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16MAKENMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QNMPIPEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CONNECTNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16DISCONNECTNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QNMPHANDSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETNMPHANDSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PEEKNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16WAITNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16TRANSACTNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CALLNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16RAWREADNMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16RAWWRITENMPIPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETNMPIPESEM(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QNMPIPESEMSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16TMRQUERYFREQ(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16TMRQUERYTIME(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16REGISTERPERFCTRS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FLATTOSEL(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SELTOFLAT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16OPENCHANGENOTIFY(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16RESETCHANGENOTIFY(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CLOSECHANGENOTIFY(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QUERYABIOSSUPPORT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CREATESPINLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16ACQUIRESPINLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16RELEASESPINLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FREESPINLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16R3EXITADDR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QUERYRESOURCESIZE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16INITIALIZEPORTHOLE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QUERYHEADERINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QUERYPROCTYPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROTECTCLOSE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROTECTFILEIO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROTECTFILELOCKS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROTECTNEWSIZE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROTECTOPEN(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROTECTQFHANDSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROTECTSETFHANDSTATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROTECTQFILEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROTECTSETFILEINFO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROTECTCHGFILEPTR(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PROTECTENUMATTRIBUTE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16LDRDIRTYWORKER(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16READQUEUE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PURGEQUEUE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CLOSEQUEUE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QUERYQUEUE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16PEEKQUEUE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16WRITEQUEUE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16OPENQUEUE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16CREATEQUEUE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMGETMEM(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMFREEMEM(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMGETSGCB(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMINITSGCB(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMSGDOPOPUP(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMSWITCH(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMSERVEAPPREQ(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16GETTIMES(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMSETTITLE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SCRUNLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMDOAPPREQ(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16STOPSESSION(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SELECTSESSION(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SCRLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SAVREDRAWWAIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SAVREDRAWUNDO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMSGENDPOPUP(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETSESSION(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETMNLOCKTIME(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16MODEUNDO(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16STARTSESSION(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMGETSTATUS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16MODEWAIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMTERMINATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMGETAPPREQ(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMINITIALIZE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMSTART(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMPARENTSWITCH(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMPAUSE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMHDEINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMPMPRESENT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMREGISTERDD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMNOTIFYDD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMNOTIFYDD2(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMOPENDD(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SMSETSESSIONTYPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         OS2BaseInit(void)
{
  return unimplemented(__FUNCTION__);
}

/* MouCalls */

USHORT __pascal MOUREGISTER(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFuns)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUDEREGISTER(VOID)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUFLUSHQUE(const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUGETPTRPOS(PTRLOC * pmouLoc, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUSETPTRPOS(const PPTRLOC pmouLoc, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUSETPTRSHAPE(const PBYTE pBuf, const PPTRSHAPE pmoupsInfo, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUGETPTRSHAPE(BYTE * pBuf, PTRSHAPE * pmoupsInfo, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUGETDEVSTATUS(USHORT * pfsDevStatus, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUGETNUMBUTTONS(USHORT * pcButtons, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUGETNUMMICKEYS(USHORT * pcMickeys, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUREADEVENTQUE(MOUEVENTINFO * pmouevEvent, const PUSHORT pfWait, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUGETNUMQUEEL(MOUQUEINFO * qmouqi, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUGETEVENTMASK(USHORT * pfsEvents, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUSETEVENTMASK(const PUSHORT pfsEvents, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUGETSCALEFACT(SCALEFACT * pmouscFactors, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUSETSCALEFACT(const PSCALEFACT pmouscFactors, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUOPEN(const PSZ pszDvrName, HMOU * phmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUCLOSE(const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUREMOVEPTR(const PNOPTRRECT pmourtRect, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUDRAWPTR(const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUSETDEVSTATUS(const PUSHORT pfsDevStatus, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUINITREAL(const PSZ str)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal MOUSYNCH(const USHORT pszDvrName)
{
  return unimplemented(__FUNCTION__);
}


//USHORT __pascal MOUGETTHRESHOLD(THRESHOLD *pthreshold, const HMOU hmou)
USHORT __pascal MOUGETTHRESHOLD(void *pthreshold, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


//USHORT __pascal MOUSETTHRESHOLD(const PTHRESHOLD pthreshold, const HMOU hmou)
USHORT __pascal MOUSETTHRESHOLD(void *pthreshold, const HMOU hmou)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         MOUGETHOTKEY(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         MOUSETHOTKEY(void)
{
  return unimplemented(__FUNCTION__);
}

USHORT __pascal         MOUSHELLINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         KBDINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         KBDSHELLINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         KBDSWITCHFGND(void)
{
  return unimplemented(__FUNCTION__);
}

USHORT __pascal         KBDLOADINSTANCE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         KBDFREE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDREGISTER(const PSZ pszModName, const PSZ pszEntryPt,
                            const ULONG FunMask)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDDEREGISTER(VOID)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDCHARIN(KBDKEYINFO * pkbci, const USHORT fWait, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDPEEK(KBDKEYINFO * pkbci, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDFLUSHBUFFER(const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDSTRINGIN(CHAR * pch, STRINGINBUF * pchIn, const USHORT fsWait, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDSETSTATUS(const PKBDINFO pkbdinfo, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDGETSTATUS(KBDINFO * pkbdinfo, const HKBD hdbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDSETCP(const USHORT usReserved, const USHORT pidCP, const HKBD hdbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDGETCP(const ULONG ulReserved, USHORT * pidCP, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDOPEN(PHKBD * hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDCLOSE(const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDGETFOCUS(const USHORT fWait, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDFREEFOCUS(const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDSYNCH(const USHORT fsWait)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDSETFGND(VOID)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDGETHWID(PKBDHWID * kbdhwid, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDSETHWID(const PKBDHWID pkbdhwid, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDXLATE(const PKBDTRANS pkbdtrans, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal KBDSETCUSTXT(const PUSHORT usCodePage, const HKBD hkbd)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOREGISTER(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIODEREGISTER(VOID)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGLOBALREG(const PSZ pszModName, const PSZ pszEntryName, const ULONG flFun1, const ULONG flFun2, const USHORT usReturn)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETANSI(USHORT * Ansi, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETCP(const USHORT Reserved, USHORT * IdCodePage, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETCURPOS(USHORT * Row, USHORT * Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETCURTYPE(VIOCURSORINFO * CursorInfo, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETMODE(VIOMODEINFO * ModeInfo, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOREADCELLSTR(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOREADCHARSTR(CHAR * CellStr, USHORT * Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSCROLLDN(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSCROLLLF(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSCROLLRT(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Columns, const PBYTE Cell, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSCROLLUP(const USHORT TopRow, const USHORT LeftCol, const USHORT BotRow, const USHORT RightCol, const USHORT Lines, const PBYTE Cell, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSETANSI(const USHORT Ansi, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSETCP(const USHORT Reserved, const USHORT IdCodePage, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSETCURPOS(const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSETCURTYPE(const PVIOCURSORINFO CursorInfo, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSETMODE(const PVIOMODEINFO ModeInfo, const HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOWRTCELLSTR(const PCHAR CellStr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOWRTCHARSTR(const PCHAR Str, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


//USHORT __pascal VIOWRTCHARSTRATT(const PCCH pch, const USHORT cb, const USHORT usRow, const USHORT usColumn, const PBYTE pAttr, const HVIO hvio)
USHORT __pascal VIOWRTCHARSTRATT(void *pch, const USHORT cb, const USHORT usRow, const USHORT usColumn, const PBYTE pAttr, const HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOWRTNATTR(const PBYTE Attr, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOWRTNCELL(const PBYTE Cell, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOWRTNCHAR(const PCHAR Char, const USHORT Count, const USHORT Row, const USHORT Column, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOWRTTTY(const PCHAR Str, const USHORT Count, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETSTATE(const PVOID pState, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSETSTATE(const PVOID pState, const HVIO Handle)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETCONFIG(const USHORT ConfigId, VIOCONFIGINFO * vioin, const HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOPOPUP (PUSHORT pfWait,
                         HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOENDPOPUP (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETPHYSBUF (PVIOPHYSBUF pvioPhysBuf,
                              USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOPRTSC(HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSCRLOCK (USHORT fWait,
                           PUCHAR pfNotLocked,
                           HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSCRUNLOCK (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSAVREDRAWWAIT (USHORT usRedrawInd,
                                 PUSHORT pNotifyType,
                                 USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSAVREDRAWUNDO (USHORT usOwnerInd,
                                 USHORT usKillInd,
                                 USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETFONT (PVIOFONTINFO pviofi,
                           HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSETFONT (PVIOFONTINFO pviofi,
                           HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETBUF (PULONG pLVB,
                          PUSHORT pcbLVB,
                          HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOMODEWAIT (USHORT usReqType,
                            PUSHORT pNotifyType,
                            USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOMODEUNDO (USHORT usOwnerInd,
                              USHORT usKillInd,
                              USHORT usReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSHOWBUF (USHORT offLVB,
                             USHORT cb,
                             HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOPRTSCTOGGLE (HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOCHECKCHARTYPE (PUSHORT pType,
                                 USHORT usRow,
                                 USHORT usColumn,
                                 HVIO hvio)
{
  return unimplemented(__FUNCTION__);
}


/* AVIO */


USHORT __pascal VIOASSOCIATE(HDC hdc,
                             HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOCREATEPS(PHVPS phvps,
                            SHORT sdepth,
                            SHORT swidth,
                            SHORT sFormat,
                            SHORT sAttrs,
                            HVPS hvpsReserved)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIODESTROYPS(HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIODELETESETID(LONG llcid,
                               HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETDEVICECELLSIZE(PSHORT psHeight,
                                     PSHORT psWidth,
                                     HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOGETORG(PSHORT psRow,
                          PSHORT psColumn,
                          HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOCREATELOGFONT(PFATTRS pfatattrs,
                                 LONG llcid,
                                 PSTR8 pName,
                                 HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOQUERYSETIDS(PLONG allcids,
                               PSTR8 pNames,
                               PLONG alTypes,
                               LONG lcount,
                               HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSETORG(SHORT sRow,
                          SHORT sColumn,
                          HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOQUERYFONTS(PLONG plRemfonts,
                              PFONTMETRICS afmMetrics,
                              LONG lMetricsLength,
                              PLONG plFonts,
                              PSZ pszFacename,
                              ULONG flOptions,
                              HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSETDEVICECELLSIZE(SHORT sHeight,
                                     SHORT sWidth,
                                     HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal VIOSHOWPS(SHORT sDepth,
                          SHORT sWidth,
                          SHORT soffCell,
                          HVPS hvps)
{
  return unimplemented(__FUNCTION__);
}

//.....add......

USHORT __pascal         VIOFREE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         AVS_PRTSC(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         AVS_PRTSCTOGGLE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         VIOSRFBLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         VIOSRFUNBLOCK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         VIOSAVE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         VIORESTORE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         VIOHETINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         VIOSSWSWITCH(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         VIOSHELLINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         VIOGETPSADDRESS(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         VIOQUERYCONSOLE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         VIOREDRAWSIZE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         XVIOSETCASTATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         XVIOCHECKCHARTYPE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         XVIODESTROYCA(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         XVIOCREATECA(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         XVIOGETCASTATE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         BVSMAIN(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         BVSREDRAWSIZE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         BVSGETPTRDRAWNAME(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         BKSMAIN(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         BMSMAIN(void)
{
  return unimplemented(__FUNCTION__);
}



USHORT __pascal            DOS16SMSYSINIT(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         QhKeybdHandle(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         QhMouseHandle(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         SMQueueRamSem(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         SMArray(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         SMPIDArray(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         SMInitialized(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         SMArraySize(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         BVSGLOBAL(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         SMGINSTANCE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         BVHINSTANCE(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOSPM16SEMRST(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FSRAMSEMREQUEST2(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16FSRAMSEMCLEAR2(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16SETEXTLIBPATH(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOS16QUERYEXTLIBPATH(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOSPM16SETRST(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         DOSPM16SEMCHK(void)
{
  return unimplemented(__FUNCTION__);
}


APIRET APIENTRY      DOSQPROCSTATUS(PVOID pBuf, USHORT cbBuf)
{
  return unimplemented(__FUNCTION__);
}

APIRET APIENTRY      DOSRETFORWARD(void)
{
  return unimplemented(__FUNCTION__);
}

USHORT __pascal         THK16_UNITHUNK(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         HT16_STARTUP(void)
{
  return unimplemented(__FUNCTION__);
}


USHORT __pascal         HT16_CLEANUP(void)
{
  return unimplemented(__FUNCTION__);
}

