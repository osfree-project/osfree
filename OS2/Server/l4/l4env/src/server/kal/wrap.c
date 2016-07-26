/*
 *
 */

#include <l4/os3/gcc_os2def.h>
#include <l4/os3/kal.h>
#include <l4/os3/processmgr.h>
#include <l4/os3/loader.h>

#include <stdarg.h>

void KalInit(va_list a)
{
  struct kal_init_struct *s;
  
  s = va_arg(a, struct kal_init_struct *);
  kalInit(s);
}

APIRET CDECL
KalStartApp(va_list a)
{
  char *name;
  char *pszLoadError;
  ULONG cbLoadError;

  name         = va_arg(a, char *);
  pszLoadError = va_arg(a, char *);
  cbLoadError  = va_arg(a, ULONG);
  return kalStartApp(name, pszLoadError, cbLoadError);
}

unsigned long
KalPvtLoadModule(va_list a)
{
  char *pszName;
  unsigned long cbName;
  char const *pszModname;
  os2exec_module_t *s;
  unsigned long *phmod;
  
  pszName    = va_arg(a, char *);
  cbName     = va_arg(a, unsigned long);
  pszModname = va_arg(a, char const *);
  s          = va_arg(a, os2exec_module_t *);
  phmod      = va_arg(a, unsigned long *);
  return kalPvtLoadModule(pszName,
                       cbName,
                       pszModname,
                       s,
                       phmod);
}

APIRET CDECL
KalRead(va_list a)
{
  HFILE hFile;
  PVOID pBuffer;
  ULONG cbRead;
  PULONG pcbRead;
  
  hFile   = va_arg(a, HFILE);
  pBuffer = va_arg(a, PVOID);
  cbRead  = va_arg(a, ULONG);
  pcbRead = va_arg(a, PULONG);
  return kalRead(hFile, pBuffer, cbRead, pcbRead);
}

APIRET CDECL
KalWrite(va_list a)
{
  HFILE hFile;
  PVOID pBuffer;
  ULONG cbWrite;
  PULONG pcbActual;
  
  hFile     = va_arg(a, HFILE);
  pBuffer   = va_arg(a, PVOID);
  cbWrite   = va_arg(a, ULONG);
  pcbActual = va_arg(a, PULONG);
  return kalWrite(hFile, pBuffer, cbWrite, pcbActual);
}

APIRET CDECL
KalLogWrite (va_list a)
{
  PSZ s;
  
  s = va_arg(a, PSZ);
  return kalLogWrite(s);
}

APIRET CDECL
KalFSCtl(va_list a)
{
  PVOID pData;
  ULONG cbData;
  PULONG pcbData;
  PVOID pParms;
  ULONG cbParms;
  PULONG pcbParms;
  ULONG function;
  PSZ pszRoute;
  HFILE hFile;
  ULONG method;

  pData    = va_arg(a, PVOID);
  cbData   = va_arg(a, ULONG);
  pcbData  = va_arg(a, PULONG);
  pParms   = va_arg(a, PVOID);
  cbParms  = va_arg(a, ULONG);
  pcbParms = va_arg(a, PULONG);
  function = va_arg(a, ULONG);
  pszRoute = va_arg(a, PSZ);
  hFile    = va_arg(a, HFILE);
  method   = va_arg(a, ULONG);
  return kalFSCtl(pData, cbData,
                  pcbData, pParms,
                  cbParms, pcbParms,
                  function, pszRoute,
                  hFile, method);
}

VOID CDECL
KalExit(va_list a)
{
  ULONG action;
  ULONG result;

  action = va_arg(a, ULONG);
  result = va_arg(a, ULONG);
  return kalExit(action, result);
}

APIRET CDECL
KalQueryCurrentDisk(va_list a)
{
  PULONG pdisknum;
  PULONG plogical;

  pdisknum = va_arg(a, PULONG);
  plogical = va_arg(a, PULONG);
  return kalQueryCurrentDisk(pdisknum, plogical);
}

APIRET CDECL
KalSetCurrentDir(va_list a)
{
  PSZ pszDir;

  pszDir = va_arg(a, PSZ);
  return kalSetCurrentDir(pszDir);
}

APIRET CDECL
KalSetDefaultDisk(va_list a)
{
  ULONG disknum;

  disknum = va_arg(a, ULONG);
  return kalSetDefaultDisk(disknum);
}

APIRET CDECL
KalQueryCurrentDir(va_list a)
{
  ULONG  disknum;
  BYTE   *pBuf;
  PULONG pcbBuf;

  disknum = va_arg(a, ULONG);
  pBuf    = va_arg(a, BYTE *);
  pcbBuf  = va_arg(a, PULONG);
  return kalQueryCurrentDir(disknum, pBuf, pcbBuf);
}

APIRET CDECL
KalLoadModule(va_list a)
{
  PSZ pszName;
  ULONG cbName;
  char const *pszModname;
  PULONG phmod;

  pszName    = va_arg(a, PSZ);
  cbName     = va_arg(a, ULONG);
  pszModname = va_arg(a, char const *);
  phmod      = va_arg(a, PULONG);
  return kalLoadModule(pszName,
                       cbName,
                       pszModname,
                       phmod);
}

APIRET CDECL
KalQueryProcAddr(va_list a)
{
  ULONG hmod;
  ULONG ordinal;
  PSZ pszName;
  void  **ppfn;

  hmod    = va_arg(a, ULONG);
  ordinal = va_arg(a, ULONG);
  pszName = va_arg(a, const PSZ);
  ppfn    = va_arg(a, void **);
  return kalQueryProcAddr(hmod,
                          ordinal,
                          pszName,
                          ppfn);
}

APIRET CDECL
KalQueryProcType(va_list a)
{
  HMODULE hmod;
  ULONG ordinal;
  PSZ pszName;
  PULONG pulProcType;

  hmod        = va_arg(a, HMODULE);
  ordinal     = va_arg(a, ULONG);
  pszName     = va_arg(a, PSZ);
  pulProcType = va_arg(a, PULONG);
  return kalQueryProcType(hmod,
                          ordinal,
		          pszName,
		          pulProcType);
}

APIRET CDECL
KalQueryAppType(va_list a)
{
  PSZ pszName;
  PULONG pFlags;

  pszName = va_arg(a, PSZ);
  pFlags  = va_arg(a, PULONG);
  return kalQueryAppType(pszName, pFlags);
}

APIRET CDECL
KalExecPgm(va_list a)
{
  char *pObjname;
  long cbObjname;
  unsigned long execFlag;
  char *pArg;
  char *pEnv;
  struct _RESULTCODES *pRes;
  char *pName;

  pObjname  = va_arg(a, char *);
  cbObjname = va_arg(a, long);
  execFlag  = va_arg(a, unsigned long);
  pArg      = va_arg(a, char *);
  pEnv      = va_arg(a, char *);
  pRes      = va_arg(a, struct _RESULTCODES *);
  pName     = va_arg(a, char *);
  return kalExecPgm(pObjname, cbObjname,
                    execFlag, pArg, pEnv,
                    pRes, pName);
}

APIRET CDECL
KalError(va_list a)
{
  ULONG error;

  error = va_arg(a, ULONG);
  return kalError(error);
}

APIRET CDECL
KalAllocMem(va_list a)
{
  PVOID *ppb;
  ULONG cb;
  ULONG flags;

  ppb   = va_arg(a, PVOID *);
  cb    = va_arg(a, ULONG);
  flags = va_arg(a, ULONG);
  return kalAllocMem(ppb, cb, flags);
}

APIRET CDECL
KalFreeMem(va_list a)
{
  PVOID pb;

  pb = va_arg(a, PVOID);
  return kalFreeMem(pb);
}

APIRET CDECL
KalResetBuffer(va_list a)
{
  HFILE handle;
  
  handle = va_arg(a, HFILE);
  return kalResetBuffer(handle);
}

APIRET CDECL
KalSetFilePtrL(va_list a)
{
  HFILE handle;
  LONGLONG ib;
  ULONG method;
  PULONGLONG ibActual;

  handle   = va_arg(a, HFILE);
  ib       = va_arg(a, LONGLONG);
  method   = va_arg(a, ULONG);
  ibActual = va_arg(a, PULONGLONG);
  return kalSetFilePtrL(handle, ib,
	                method, ibActual);
}
	      
APIRET CDECL
KalClose(va_list a)
{
  HFILE handle;

  handle = va_arg(a, HFILE);
  return kalClose(handle);
}

APIRET CDECL
KalSetMaxFH(va_list a)
{
  ULONG cFH;

  cFH = va_arg(a, ULONG);
  return kalSetMaxFH(cFH);
}

APIRET CDECL
KalSetRelMaxFH(va_list a)
{
  PLONG pcbReqCount;
  PULONG pcbCurMaxFH;

  pcbReqCount = va_arg(a, PLONG);
  pcbCurMaxFH = va_arg(a, PULONG); 
  return kalSetRelMaxFH(pcbReqCount, pcbCurMaxFH);
}

APIRET CDECL
KalFindFirst(va_list a)
{
  char  *pszFileSpec;
  HDIR  *phDir;
  ULONG flAttribute;
  PVOID pFindBuf;
  LONG  cbBuf;
  ULONG *pcFileNames;
  ULONG ulInfolevel;

  pszFileSpec = va_arg(a, char *);
  phDir       = va_arg(a, HDIR *);
  flAttribute = va_arg(a, ULONG);
  pFindBuf    = va_arg(a, PVOID);
  cbBuf       = va_arg(a, LONG);
  pcFileNames = va_arg(a, ULONG *);
  ulInfolevel = va_arg(a, ULONG);
  return kalFindFirst(pszFileSpec, phDir,
             flAttribute, pFindBuf, cbBuf,
             pcFileNames, ulInfolevel);
}

APIRET CDECL
KalFindNext(va_list a)
{
  HDIR  hDir;
  PVOID pFindBuf;
  ULONG cbBuf;
  ULONG *pcFileNames;

  hDir        = va_arg(a, HDIR);
  pFindBuf    = va_arg(a, PVOID);
  cbBuf       = va_arg(a, ULONG);
  pcFileNames = va_arg(a, ULONG *); 
  return kalFindNext(hDir, pFindBuf, cbBuf, pcFileNames);
}

APIRET CDECL
KalFindClose(va_list a)
{
  HDIR hDir;

  hDir = va_arg(a, HDIR);
  return kalFindClose(hDir);
}

APIRET CDECL
KalQueryFHState(va_list a)
{
  HFILE hFile;
  PULONG pMode;

  hFile = va_arg(a, HFILE);
  pMode = va_arg(a, PULONG);
  return kalQueryFHState(hFile, pMode);
}

APIRET CDECL
KalSetFHState(va_list a)
{
  HFILE hFile;
  ULONG pMode;

  hFile = va_arg(a, HFILE);
  pMode = va_arg(a, ULONG);
  return kalSetFHState(hFile, pMode);
}

APIRET CDECL
KalQueryFileInfo(va_list a)
{
  HFILE hf;
  ULONG ulInfoLevel;
  char *pInfo;
  ULONG cbInfoBuf;

  hf          = va_arg(a, HFILE);
  ulInfoLevel = va_arg(a, ULONG);
  pInfo       = va_arg(a, char *);
  cbInfoBuf   = va_arg(a, ULONG);
  return kalQueryFileInfo(hf, ulInfoLevel, pInfo, cbInfoBuf);
}

APIRET CDECL
KalQueryPathInfo(va_list a)
{
  PSZ pszPathName;
  ULONG ulInfoLevel;
  PVOID pInfo;
  ULONG cbInfoBuf;

  pszPathName = va_arg(a, PSZ);
  ulInfoLevel = va_arg(a, ULONG);
  pInfo       = va_arg(a, PVOID);
  cbInfoBuf   = va_arg(a, ULONG);
  return kalQueryPathInfo(pszPathName, ulInfoLevel,
                 pInfo, cbInfoBuf);
}

APIRET CDECL
KalSetFileSizeL(va_list a)
{
  HFILE hFile;
  long long cbSize;

  hFile  = va_arg(a, HFILE);
  cbSize = va_arg(a, long long);
  return kalSetFileSizeL(hFile, cbSize);
}

APIRET CDECL
KalAllocSharedMem(va_list a)
{
  PPVOID ppb;
  PSZ    pszName;
  ULONG  cb;
  ULONG  flags;

  ppb     = va_arg(a, PPVOID);
  pszName = va_arg(a, PSZ);
  cb      = va_arg(a, ULONG);
  flags   = va_arg(a, ULONG);
  return kalAllocSharedMem(ppb, pszName,
		           cb, flags);
}

APIRET CDECL
KalGetSharedMem(va_list a)
{
  PVOID pb;
  ULONG flag;

  pb     = va_arg(a, PVOID);
  flag   = va_arg(a, ULONG);
  return kalGetSharedMem(pb, flag);
}

APIRET CDECL
KalGetNamedSharedMem(va_list a)
{
  PPVOID ppb;
  PSZ pszName;
  ULONG flag;

  ppb     = va_arg(a, PPVOID);
  pszName = va_arg(a, PSZ);
  flag    = va_arg(a, ULONG);
  return kalGetNamedSharedMem(ppb, pszName, flag);
}

APIRET CDECL
KalGiveSharedMem(va_list a)
{
  PVOID pb;
  PID   pid;
  ULONG flag;

  pb     = va_arg(a, PVOID);
  pid    = va_arg(a, PID);
  flag   = va_arg(a, ULONG);
  return kalGiveSharedMem(pb, pid, flag);
}

APIRET CDECL
KalCreateDir(va_list a)
{
  PSZ pszDirName;
  PEAOP2 peaop2;
 
  pszDirName = va_arg(a, PSZ);
  peaop2     = va_arg(a, PEAOP2);
  return kalCreateDir(pszDirName, peaop2);
}

APIRET CDECL
KalDupHandle(va_list a)
{
  HFILE hFile;
  HFILE *phFile2;
  
  hFile   = va_arg(a, HFILE);
  phFile2 = va_arg(a, HFILE *);
  return kalDupHandle(hFile, phFile2);
}

APIRET CDECL
KalDelete(va_list a)
{
  PSZ pszFileName;

  pszFileName = va_arg(a, PSZ);
  return kalDelete(pszFileName);
}

APIRET CDECL
KalForceDelete(va_list a)
{
  PSZ pszFileName;

  pszFileName = va_arg(a, PSZ);
  return kalForceDelete(pszFileName);
}

APIRET CDECL
KalDeleteDir(va_list a)
{
  PSZ pszDirName;

  pszDirName = va_arg(a, PSZ);
  return kalDeleteDir(pszDirName);
}

APIRET CDECL
KalGetInfoBlocks(va_list a)
{
  PTIB *pptib;
  PPIB *pppib;

  pptib = va_arg(a, PTIB *);
  pppib = va_arg(a, PPIB *);
  return kalGetInfoBlocks(pptib, pppib);
}

APIRET CDECL
KalMove(va_list a)
{
  PSZ pszOld;
  PSZ pszNew;
  
  pszOld = va_arg(a, PSZ);
  pszNew = va_arg(a, PSZ);
  return kalMove(pszOld, pszNew);
}

APIRET CDECL
KalOpenL (va_list a)
{
  PSZ pszFileName;
  HFILE *phFile;
  ULONG *pulAction;
  LONGLONG cbFile;
  ULONG ulAttribute;
  ULONG fsOpenFlags;
  ULONG fsOpenMode;
  PEAOP2 peaop2;

  pszFileName = va_arg(a, PSZ);
  phFile      = va_arg(a, HFILE *);
  pulAction   = va_arg(a, ULONG *);
  cbFile      = va_arg(a, LONGLONG);
  ulAttribute = va_arg(a, ULONG);
  fsOpenFlags = va_arg(a, ULONG);
  fsOpenMode  = va_arg(a, ULONG);
  peaop2      = va_arg(a, PEAOP2);
  return kalOpenL (pszFileName, phFile, pulAction,
	  cbFile, ulAttribute, fsOpenFlags,
	  fsOpenMode, peaop2);
}
	  
APIRET CDECL
KalQueryHType(va_list a)
{
  HFILE handle;
  PULONG pType;
  PULONG pAttr;

  handle = va_arg(a, HFILE);
  pType  = va_arg(a, PULONG);
  pAttr  = va_arg(a, PULONG);
  return kalQueryHType(handle, pType, pAttr);
}

APIRET CDECL
KalQueryDBCSEnv(va_list a)
{
  ULONG cb;
  COUNTRYCODE *pcc;
  PBYTE pBuf;

  cb   = va_arg(a, ULONG);
  pcc  = va_arg(a, COUNTRYCODE *);
  pBuf = va_arg(a, PBYTE);
  return kalQueryDBCSEnv(cb, pcc, pBuf);
}

APIRET CDECL
KalQueryCp(va_list a)
{
  ULONG cb;
  PULONG arCP;
  PULONG pcCP;

  cb   = va_arg(a, ULONG);
  arCP = va_arg(a, PULONG);
  pcCP = va_arg(a, PULONG);
  return kalQueryCp(cb, arCP, pcCP);
}

APIRET CDECL
KalQueryMem(va_list a)
{
  PVOID  pb;
  PULONG pcb;
  PULONG pflags;

  pb     = va_arg(a, PVOID);
  pcb    = va_arg(a, PULONG);
  pflags = va_arg(a, PULONG);
  return kalQueryMem(pb, pcb, pflags);
}

APIRET CDECL
KalQueryModuleName(va_list a)
{
  ULONG hmod;
  ULONG cbBuf;
  PBYTE pBuf;

  hmod   = va_arg(a, ULONG);
  cbBuf  = va_arg(a, ULONG);
  pBuf   = va_arg(a, PBYTE);
  return kalQueryModuleName(hmod, cbBuf, pBuf);
}

APIRET CDECL
KalQueryModuleHandle(va_list a)
{
  PCSZ pszModname;
  PULONG phmod;

  pszModname = va_arg(a, PCSZ);
  phmod      = va_arg(a, PULONG);
  return kalQueryModuleHandle(pszModname, phmod);
}

APIRET CDECL
KalSleep(va_list a)
{
  ULONG ms;

  ms = va_arg(a, ULONG);
  return kalSleep(ms);
}

APIRET CDECL
KalSetMem(va_list a)
{
  PVOID pb;
  ULONG cb;
  ULONG flags;

  pb    = va_arg(a, PVOID);
  cb    = va_arg(a, ULONG);
  flags = va_arg(a, ULONG);
  return kalSetMem(pb, cb, flags);
}

APIRET CDECL
KalOpenEventSem(va_list a)
{
  PSZ pszName;
  PHEV phev;

  pszName = va_arg(a, PSZ);
  phev    = va_arg(a, PHEV);
  return kalOpenEventSem(pszName, phev);
}

APIRET CDECL
KalCloseEventSem(va_list a)
{
  HEV hev;

  hev = va_arg(a, HEV);
  return kalCloseEventSem(hev);
}

APIRET CDECL
KalCreateEventSem(va_list a)
{
  PSZ pszName;
  PHEV phev;
  ULONG flags;
  BOOL32 fState;

  pszName = va_arg(a, PSZ);
  phev    = va_arg(a, PHEV);
  flags   = va_arg(a, ULONG);
  fState  = va_arg(a, BOOL32);
  return kalCreateEventSem(pszName, phev, flags, fState);
}

APIRET CDECL
KalCreateThread(va_list a)
{
  PTID ptid;
  PFNTHREAD pfn;
  ULONG param;
  ULONG flag;
  ULONG cbStack;

  ptid    = va_arg(a, PTID);
  pfn     = va_arg(a, PFNTHREAD);
  param   = va_arg(a, ULONG);
  flag    = va_arg(a, ULONG);
  cbStack = va_arg(a, ULONG);
  return kalCreateThread(ptid, pfn, param, flag, cbStack);
}

APIRET CDECL
KalSuspendThread(va_list a)
{
  TID tid;

  tid = va_arg(a, TID);
  return kalSuspendThread(tid);
}

APIRET CDECL
KalResumeThread(va_list a)
{
  TID tid;

  tid = va_arg(a, TID);
  return kalResumeThread(tid);
}

APIRET CDECL
KalWaitThread(va_list a)
{
  PTID ptid;
  ULONG option;

  ptid    = va_arg(a, PTID);
  option  = va_arg(a, ULONG);
  return kalWaitThread(ptid, option);
}

APIRET CDECL
KalKillThread(va_list a)
{
  TID tid;

  tid = va_arg(a, TID);
  return kalKillThread(tid);
}

APIRET CDECL
KalGetTID(va_list a)
{
  TID *ptid;

  ptid = va_arg(a, TID *);
  return kalGetTID(ptid);
}

APIRET CDECL
KalGetPID(va_list a)
{
  PID *ppid;

  ppid = va_arg(a, PID *);
  return kalGetPID(ppid);
}

APIRET CDECL
KalGetL4ID(va_list a)
{
  PID pid;
  TID tid;
  l4_threadid_t *id;

  pid  = va_arg(a, PID);
  tid  = va_arg(a, TID);
  id   = va_arg(a, l4thread_t *);
  return kalGetL4ID(pid, tid, id);
}

APIRET CDECL
KalGetTIDL4(va_list a)
{
  l4_threadid_t id;
  TID *ptid;

  id   = va_arg(a, l4_threadid_t);
  ptid = va_arg(a, TID *);
  return kalGetTIDL4(id, ptid);
}

APIRET CDECL
KalNewTIB(va_list a)
{
  PID pid;
  TID tid;
  l4thread_t id;

  pid = va_arg(a, PID);
  tid = va_arg(a, TID);
  id  = va_arg(a, l4thread_t);
  return kalNewTIB(pid, tid, id);
}

APIRET CDECL
KalDestroyTIB(va_list a)
{
  PID pid;
  TID tid;

  pid = va_arg(a, PID);
  tid = va_arg(a, TID);
  return kalDestroyTIB(pid, tid);
}

APIRET CDECL
KalGetTIB(va_list a)
{
  PTIB *ptib;

  ptib = va_arg(a, PTIB *);
  return kalGetTIB(ptib);
}

APIRET CDECL
KalGetPIB(va_list a)
{
  PPIB *ppib;

  ppib = va_arg(a, PPIB *);
  return kalGetPIB(ppib);
}
