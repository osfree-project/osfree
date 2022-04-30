/*****************************************************************************
    dosfilemgrcommon.h
    (C) 2004-2010 osFree project

    WARNING! Automaticaly generated file! Don't edit it manually!
*****************************************************************************/

#ifndef __DOSFILEMGRCOMMON_H__
#define __DOSFILEMGRCOMMON_H__

#ifdef __cplusplus
   extern "C" {
#endif

#ifndef NULL
#define NULL  0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef EXPENTRY
#define EXPENTRY  _System
#endif

#ifndef APIENTRY
#define APIENTRY  _System
#endif

#ifndef FAR
#define FAR
#endif

#ifndef NEAR
#define NEAR
#endif

#define FILE_BEGIN 0x0000
#define FILE_CURRENT 0x0001
#define FILE_END 0x0002
#define FILE_SECTOR 0x8000
#define LISTIO_READ 0x0004
#define LISTIO_WRITE 0x0008
#define LISTIO_ORDERED 0x0001
#define LISTIO_UNORDERED 0x0002
#define HDIR_SYSTEM 1
#define HDIR_CREATE ( - 1 )
#define DCPY_EXISTING 0x0001
#define DCPY_APPEND 0x0002
#define DCPY_FAILEAS 0x0004
#define FILE_NORMAL 0x0000
#define FILE_READONLY 0x0001
#define FILE_HIDDEN 0x0002
#define FILE_SYSTEM 0x0004
#define FILE_DIRECTORY 0x0010
#define FILE_ARCHIVED 0x0020
#define FILE_IGNORE 0x10000
#define MUST_HAVE_READONLY ( ( FILE_READONLY << 8 ) | FILE_READONLY )
#define MUST_HAVE_HIDDEN ( ( FILE_HIDDEN << 8 ) | FILE_HIDDEN )
#define MUST_HAVE_SYSTEM ( ( FILE_SYSTEM << 8 ) | FILE_SYSTEM )
#define MUST_HAVE_DIRECTORY ( ( FILE_DIRECTORY << 8 ) | FILE_DIRECTORY )
#define MUST_HAVE_ARCHIVED ( ( FILE_ARCHIVED << 8 ) | FILE_ARCHIVED )
#define FILE_EXISTED 0x0001
#define FILE_CREATED 0x0002
#define FILE_TRUNCATED 0x0003
#define FILE_OPEN 0x0001
#define FILE_TRUNCATE 0x0002
#define FILE_CREATE 0x0010
#define OPEN_ACTION_FAIL_IF_EXISTS 0x0000
#define OPEN_ACTION_OPEN_IF_EXISTS 0x0001
#define OPEN_ACTION_REPLACE_IF_EXISTS 0x0002
#define OPEN_ACTION_FAIL_IF_NEW 0x0000
#define OPEN_ACTION_CREATE_IF_NEW 0x0010
#define OPEN_ACCESS_READONLY 0x0000
#define OPEN_ACCESS_WRITEONLY 0x0001
#define OPEN_ACCESS_READWRITE 0x0002
#define OPEN_SHARE_DENYREADWRITE 0x0010
#define OPEN_SHARE_DENYWRITE 0x0020
#define OPEN_SHARE_DENYREAD 0x0030
#define OPEN_SHARE_DENYNONE 0x0040
#define OPEN_FLAGS_NOINHERIT 0x0080
#define OPEN_FLAGS_NO_LOCALITY 0x0000
#define OPEN_FLAGS_SEQUENTIAL 0x0100
#define OPEN_FLAGS_RANDOM 0x0200
#define OPEN_FLAGS_RANDOMSEQUENTIAL 0x0300
#define OPEN_FLAGS_NO_CACHE 0x1000
#define OPEN_FLAGS_FAIL_ON_ERROR 0x2000
#define OPEN_FLAGS_WRITE_THROUGH 0x4000
#define OPEN_FLAGS_DASD 0x8000
#define OPEN_FLAGS_NONSPOOLED 0x00040000
#define OPEN_SHARE_DENYLEGACY 0x10000000
#define OPEN_FLAGS_PROTECTED_HANDLE 0x40000000
#define SEARCH_PATH 0x0000
#define SEARCH_CUR_DIRECTORY 0x0001
#define SEARCH_ENVIRONMENT 0x0002
#define SEARCH_IGNORENETERRS 0x0004
#define FIL_STANDARD 1
#define FIL_QUERYEASIZE 2
#define FIL_QUERYEASFROMLIST 3
#define FIL_STANDARDL 11
#define FIL_QUERYEASIZEL 12
#define FIL_QUERYEASFROMLISTL 13
#define FIL_QUERYFULLNAME 5
#define FS_ATTACH 0
#define FS_DETACH 1
#define FS_SPOOLATTACH 2
#define FS_SPOOLDETACH 3
#define FSCTL_HANDLE 1
#define FSCTL_PATHNAME 2
#define FSCTL_FSDNAME 3
#define FSCTL_ERROR_INFO 1
#define FSCTL_MAX_EASIZE 2
#define FSCTL_GET_NEXT_ROUTE_NAME 3
#define FSCTL_DAEMON_QUERY 4
#define FSCTL_QUERY_COMPLETE 0x0000
#define FSCTL_QUERY_AGAIN 0x0001
typedef struct _EASIZEBUF {
USHORT cbMaxEASize;
ULONG cbMaxEAListSize;
} EASIZEBUF;
typedef EASIZEBUF PEASIZEBUF;
typedef struct _ROUTENAMEBUF {
ULONG hRouteHandle;
UCHAR szRouteName;
} ROUTENAMEBUF;
typedef ROUTENAMEBUF * PROUTENAMEBUF;
typedef struct _FSDTHREAD {
USHORT usFunc;
USHORT usStackSize;
ULONG ulPriorityClass;
LONG lPriorityLevel;
} FSDTHREAD;
typedef struct _FSDDAEMON {
USHORT usNumThreads;
USHORT usMoreFlag;
USHORT usCallInstance;
FSDTHREAD tdThrds[16];
} FSDDAEMON;
#define FSAIL_QUERYNAME 1
#define FSAIL_DEVNUMBER 2
#define FSAIL_DRVNUMBER 3
#define FSAT_CHARDEV 1
#define FSAT_PSEUDODEV 2
#define FSAT_LOCALDRV 3
#define FSAT_REMOTEDRV 4
typedef struct _FSQBUFFER {
USHORT iType;
USHORT cbName;
UCHAR szName[1];
USHORT cbFSDName;
UCHAR szFSDName[1];
USHORT cbFSAData;
UCHAR rgFSAData[1];
} FSQBUFFER;
typedef FSQBUFFER * PFSQBUFFER;
typedef struct _FSQBUFFER2 {
USHORT iType;
USHORT cbName;
USHORT cbFSDName;
USHORT cbFSAData;
UCHAR szName[1];
UCHAR szFSDName[1];
UCHAR rgFSAData[1];
} FSQBUFFER2;
typedef FSQBUFFER2 * PFSQBUFFER2;
typedef struct _SPOOLATTACH {
USHORT hNmPipe;
ULONG ulKey;
} SPOOLATTACH;
typedef SPOOLATTACH * PSPOOLATTACH;
#define FSIL_ALLOC 1
#define FSIL_VOLSER 2
#define FHT_DISKFILE 0x0000
#define FHT_CHRDEV 0x0001
#define FHT_PIPE 0x0002
#define FHB_DSKREMOTE 0x8000
#define FHB_CHRDEVREMOTE 0x8000
#define FHB_PIPEREMOTE 0x8000
typedef struct _FTIME {
USHORT twosecs;
USHORT minutes;
USHORT hours;
} FTIME;
typedef FTIME * PFTIME;
typedef struct _FDATE {
USHORT day;
USHORT month;
USHORT year;
} FDATE;
typedef FDATE * PFDATE;
typedef struct _VOLUMELABEL {
BYTE cch;
CHAR szVolLabel[12];
} VOLUMELABEL;
typedef VOLUMELABEL * PVOLUMELABEL;
typedef struct _FSINFO {
FDATE fdateCreation;
FTIME ftimeCreation;
VOLUMELABEL vol;
} FSINFO;
typedef FSINFO * PFSINFO;
#define HANDTYPE_FILE 0x0000
#define HANDTYPE_DEVICE 0x0001
#define HANDTYPE_PIPE 0x0002
#define HANDTYPE_PROTECTED 0x4000
#define HANDTYPE_NETWORK 0x8000
typedef struct _FILELOCK {
LONG lOffset;
LONG lRange;
} FILELOCK;
typedef FILELOCK * PFILELOCK;
typedef struct _FILELOCKL {
LONGLONG lOffset;
LONGLONG lRange;
} FILELOCKL;
typedef FILELOCKL * PFILELOCKL;
typedef ULONG FHLOCK;
typedef PULONG PFHLOCK;
APIRET APIENTRY DosSetFileLocks(const HFILE hFile, const PFILELOCK pflUnlock, const PFILELOCK pflLock, const ULONG timeout, const ULONG flags);
APIRET APIENTRY DosSetFileLocksL(const HFILE hFile, const PFILELOCKL pflUnlock, const PFILELOCKL pflLock, const ULONG timeout, const ULONG flags);
APIRET APIENTRY DosProtectSetFileLocks(const HFILE hFile, const PFILELOCK pflUnlock, const PFILELOCK pflLock, const ULONG timeout, const ULONG flags, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosProtectSetFileLocksL(const HFILE hFile, const PFILELOCKL pflUnlock, const PFILELOCKL pflLock, const ULONG timeout, const ULONG flags, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosCancelLockRequest(const HFILE hFile, const PFILELOCK pflLock);
APIRET APIENTRY DosCancelLockRequestL(const HFILE hFile, const PFILELOCKL pflLock);
typedef struct _FILEFINDBUF {
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
ULONG cbFile;
ULONG cbFileAlloc;
USHORT attrFile;
UCHAR cchName;
CHAR achName[CCHMAXPATHCOMP];
} FILEFINDBUF;
typedef FILEFINDBUF * PFILEFINDBUF;
typedef struct _FILEFINDBUF2 {
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
ULONG cbFile;
ULONG cbFileAlloc;
USHORT attrFile;
ULONG cbList;
UCHAR cchName;
CHAR achName[CCHMAXPATHCOMP];
} FILEFINDBUF2;
typedef FILEFINDBUF2 * PFILEFINDBUF2;
typedef struct _FILEFINDBUF3 {
ULONG oNextEntryOffset;
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
ULONG cbFile;
ULONG cbFileAlloc;
ULONG attrFile;
UCHAR cchName;
CHAR achName[CCHMAXPATHCOMP];
} FILEFINDBUF3;
typedef FILEFINDBUF3 * PFILEFINDBUF3;
typedef struct _FILEFINDBUF4 {
ULONG oNextEntryOffset;
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
ULONG cbFile;
ULONG cbFileAlloc;
ULONG attrFile;
ULONG cbList;
UCHAR cchName;
CHAR achName[CCHMAXPATHCOMP];
} FILEFINDBUF4;
typedef FILEFINDBUF4 * PFILEFINDBUF4;
typedef struct _FILEFINDBUF3L {
ULONG oNextEntryOffset;
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
LONGLONG cbFile;
LONGLONG cbFileAlloc;
ULONG attrFile;
UCHAR cchName;
CHAR achName[CCHMAXPATHCOMP];
} FILEFINDBUF3L;
typedef FILEFINDBUF3L * PFILEFINDBUF3L;
typedef struct _FILEFINDBUF4L {
ULONG oNextEntryOffset;
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
LONGLONG cbFile;
LONGLONG cbFileAlloc;
ULONG attrFile;
ULONG cbList;
UCHAR cchName;
CHAR achName[CCHMAXPATHCOMP];
} FILEFINDBUF4L;
typedef FILEFINDBUF4L * PFILEFINDBUF4L;
typedef struct _GEA {
BYTE cbName;
CHAR szName[1];
} GEA;
typedef GEA * PGEA;
typedef struct _GEALIST {
ULONG cbList;
GEA list[1];
} GEALIST;
typedef GEALIST * PGEALIST;
typedef struct _FEA {
BYTE fEA;
BYTE cbName;
USHORT cbValue;
} FEA;
typedef FEA * PFEA;
#define FEA_NEEDEA 0x80
typedef struct _FEALIST {
ULONG cbList;
FEA list[1];
} FEALIST;
typedef FEALIST * PFEALIST;
typedef struct _EAOP {
PGEALIST fpGEAList;
PFEALIST fpFEAList;
ULONG oError;
} EAOP;
typedef EAOP * PEAOP;
typedef struct _FEA2 {
ULONG oNextEntryOffset;
BYTE fEA;
BYTE cbName;
USHORT cbValue;
CHAR szName[1];
} FEA2;
typedef FEA2 * PFEA2;
typedef struct _FEA2LIST {
ULONG cbList;
FEA2 list[1];
} FEA2LIST;
typedef FEA2LIST * PFEA2LIST;
typedef struct _GEA2 {
ULONG oNextEntryOffset;
BYTE cbName;
CHAR szName[1];
} GEA2;
typedef GEA2 * PGEA2;
typedef struct _GEA2LIST {
ULONG cbList;
GEA2 list[1];
} GEA2LIST;
typedef GEA2LIST * PGEA2LIST;
typedef struct _EAOP2 {
PGEA2LIST fpGEA2List;
PFEA2LIST fpFEA2List;
ULONG oError;
} EAOP2;
typedef EAOP2 * PEAOP2;
#define EAT_BINARY 0xFFFE
#define EAT_ASCII 0xFFFD
#define EAT_BITMAP 0xFFFB
#define EAT_METAFILE 0xFFFA
#define EAT_ICON 0xFFF9
#define EAT_EA 0xFFEE
#define EAT_MVMT 0xFFDF
#define EAT_MVST 0xFFDE
#define EAT_ASN1 0xFFDD
typedef struct _LISTIO {
HFILE hFile;
ULONG CmdFlag;
LONG Offset;
PVOID pBuffer;
ULONG NumBytes;
ULONG Actual;
ULONG RetCode;
ULONG Reserved;
ULONG Reserved2[3];
ULONG Reserved3[2];
} LISTIO;
typedef LISTIO * PLISTIO;
typedef struct _LISTIOL {
HFILE hFile;
ULONG CmdFlag;
LONGLONG Offset;
PVOID pBuffer;
ULONG NumBytes;
ULONG Actual;
ULONG RetCode;
ULONG Reserved;
ULONG Reserved2[3];
ULONG Reserved3[2];
} LISTIOL;
typedef LISTIOL * PLISTIOL;
APIRET APIENTRY DosOpen(const PCSZ pszFileName, HFILE * pHf, ULONG * pulAction, const ULONG cbFile, const ULONG ulAttribute, const ULONG fsOpenFlags, const ULONG fsOpenMode, EAOP2 * peaop2);
APIRET APIENTRY DosOpenL(const PCSZ pszFileName, HFILE * phf, ULONG * pulAction, const LONGLONG cbFile, const ULONG ulAttribute, const ULONG fsOpenFlags, const ULONG fsOpenMode, EAOP2 * peaop2);
APIRET APIENTRY DosProtectOpen(const PCSZ pszFileName, HFILE * phf, ULONG * pulAction, const ULONG cbFile, const ULONG ulAttribute, const ULONG fsOpenFlags, const ULONG fsOpenMode, EAOP2 * peaop2, FHLOCK * pfhFileHandleLockID);
APIRET APIENTRY DosProtectOpenL(const PCSZ pszFileName, HFILE * phf, ULONG * pulAction, const LONGLONG cbFile, const ULONG ulAttribute, const ULONG fsOpenFlags, const ULONG fsOpenMode, EAOP2 * peaop2, FHLOCK * pfhFileHandleLockID);
APIRET APIENTRY DosClose(const HFILE hFile);
APIRET APIENTRY DosProtectClose(const HFILE hFile, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosRead(const HFILE hFile, const PVOID pBuffer, const ULONG cbRead, ULONG * pcbActual);
APIRET APIENTRY DosProtectRead(const HFILE hFile, const PVOID pBuffer, const ULONG cbRead, ULONG * pcbActual, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosWrite(const HFILE hFile, const PVOID pBuffer, const ULONG cbWrite, ULONG * pcbActual);
APIRET APIENTRY DosProtectWrite(const HFILE hFile, const PVOID pBuffer, const ULONG cbWrite, ULONG * pcbActual, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosListIO(const ULONG ulCmdMode, const ULONG ulNumentries, LISTIO * pListIO);
typedef struct _FILESTATUS {
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
ULONG cbFile;
ULONG cbFileAlloc;
USHORT attrFile;
} FILESTATUS;
typedef FILESTATUS * PFILESTATUS;
typedef struct _FILESTATUS2 {
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
ULONG cbFile;
ULONG cbFileAlloc;
USHORT attrFile;
ULONG cbList;
} FILESTATUS2;
typedef FILESTATUS2 * PFILESTATUS2;
typedef struct _FILESTATUS3 {
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
ULONG cbFile;
ULONG cbFileAlloc;
ULONG attrFile;
} FILESTATUS3;
typedef FILESTATUS3 * PFILESTATUS3;
typedef struct _FILESTATUS4 {
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
ULONG cbFile;
ULONG cbFileAlloc;
ULONG attrFile;
ULONG cbList;
} FILESTATUS4;
typedef FILESTATUS4 * PFILESTATUS4;
typedef struct _FILESTATUS3L {
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
LONGLONG cbFile;
LONGLONG cbFileAlloc;
ULONG attrFile;
} FILESTATUS3L;
typedef FILESTATUS3L * PFILESTATUS3L;
typedef struct _FILESTATUS4L {
FDATE fdateCreation;
FTIME ftimeCreation;
FDATE fdateLastAccess;
FTIME ftimeLastAccess;
FDATE fdateLastWrite;
FTIME ftimeLastWrite;
LONGLONG cbFile;
LONGLONG cbFileAlloc;
ULONG attrFile;
ULONG cbList;
} FILESTATUS4L;
typedef FILESTATUS4L * PFILESTATUS4L;
typedef struct _FSALLOCATE {
ULONG idFileSystem;
ULONG cSectorUnit;
ULONG cUnit;
ULONG cUnitAvail;
USHORT cbSector;
} FSALLOCATE;
typedef FSALLOCATE * PFSALLOCATE;
typedef LHANDLE HDIR;
typedef HDIR * PHDIR;
APIRET APIENTRY DosDelete(const PCSZ pszFile);
APIRET APIENTRY DosForceDelete(const PCSZ pszFile);
APIRET APIENTRY DosDupHandle(const HFILE hFile, HFILE * pHfile);
APIRET APIENTRY DosQueryFHState(const HFILE hFile, ULONG * pMode);
APIRET APIENTRY DosProtectQueryFHState(const HFILE hFile, ULONG * pMode, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosSetFHState(const HFILE hFile, const ULONG mode);
APIRET APIENTRY DosSetFHandState(const HFILE hFile, const ULONG mode);
APIRET APIENTRY DosProtectSetFHState(const HFILE hFile, const ULONG mode, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosQueryHType(const HFILE hFile, ULONG * pType, ULONG * pAttr);
APIRET APIENTRY DosFindFirst(const PCSZ pszFileSpec, HDIR * phdir, const ULONG flAttribute, const PVOID pfindbuf, const ULONG cbBuf, ULONG * pcFileNames, const ULONG ulInfoLevel);
APIRET APIENTRY DosFindNext(const HDIR hDir, const PVOID pfindbuf, const ULONG cbfindbuf, ULONG * pcFilenames);
APIRET APIENTRY DosFindClose(const HDIR hDir);
APIRET APIENTRY DosFSAttach(const PCSZ pszDevice, const PCSZ pszFilesystem, const PVOID pData, const ULONG cbData, const ULONG flag);
APIRET APIENTRY DosQueryFSAttach(const PCSZ pszDeviceName, const ULONG ulOrdinal, const ULONG ulFSAInfoLevel, FSQBUFFER2 * pfsqb, ULONG * pcbBuffLength);
APIRET APIENTRY DosFSCtl(const PVOID pData, const ULONG cbData, ULONG * pcbData, const PVOID pParms, const ULONG cbParms, ULONG * pcbParms, const ULONG xfunction, const PCSZ pszRoute, const HFILE hFile, const ULONG method);
APIRET APIENTRY DosSetFileSize(const HFILE hFile, const ULONG cbSize);
APIRET APIENTRY DosSetFileSizeL(const HFILE hFile, const LONGLONG cbSize);
APIRET APIENTRY DosProtectSetFileSize(const HFILE hFile, const ULONG cbSize, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosProtectSetFileSizeL(const HFILE hFile, const LONGLONG cbSize, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosResetBuffer(const HFILE hFile);
APIRET APIENTRY DosBufReset(const HFILE hFile);
APIRET APIENTRY DosSetFilePtr(const HFILE hFile, const LONG ib, const ULONG method, ULONG * ibActual);
APIRET APIENTRY DosSetFilePtrL(const HFILE hFile, const LONGLONG ib, const ULONG method, ULONGLONG * ibActual);
APIRET APIENTRY DosProtectSetFilePtr(const HFILE hFile, const LONG ib, const ULONG method, ULONG * ibActual, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosProtectSetFilePtrL(const HFILE hFile, const LONGLONG ib, const ULONG method, LONGLONG * ibActual, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosListIOL(const LONG CmdMODE, const LONG NumEntries, LISTIOL * pListIO);
APIRET APIENTRY DosMove(const PCSZ pszOld, const PCSZ pszNew);
APIRET APIENTRY DosCopy(const PCSZ pszOld, const PCSZ pszNew, const ULONG option);
APIRET APIENTRY DosEditName(const ULONG metalevel, const PCSZ pszSource, const PCSZ pszEdit, BYTE * pszTarget, const ULONG cbTarget);
APIRET APIENTRY DosCreateDir(const PCSZ pszDirName, const PEAOP2 peaop2);
APIRET APIENTRY DosDeleteDir(const PCSZ pszDir);
APIRET APIENTRY DosSetDefaultDisk(const ULONG disknum);
APIRET APIENTRY DosSelectDisk(const ULONG disknum);
APIRET APIENTRY DosQueryCurrentDisk(ULONG * pdisknum, ULONG * plogical);
APIRET APIENTRY DosSetCurrentDir(const PCSZ pszDir);
APIRET APIENTRY DosQueryCurrentDir(const ULONG disknum, BYTE * pBuf, ULONG * pcbBuf);
APIRET APIENTRY DosQueryFSInfo(const ULONG disknum, const ULONG infolevel, const PVOID pBuf, const ULONG cbBuf);
APIRET APIENTRY DosSetFSInfo(const ULONG disknum, const ULONG infolevel, const PVOID pBuf, const ULONG cbBuf);
APIRET APIENTRY DosQueryVerify(BOOL32 * pBool);
APIRET APIENTRY DosSetVerify(const BOOL32 vstate);
APIRET APIENTRY DosSetMaxFH(const ULONG cFH);
APIRET APIENTRY DosSetRelMaxFH(const PLONG pcbReqCount, ULONG * pcbCurMaxFH);
APIRET APIENTRY DosQueryFileInfo(const HFILE hf, const ULONG ulInfoLevel, const PVOID pInfo, const ULONG cbInfoBuf);
APIRET APIENTRY DosProtectQueryFileInfo(const HFILE hf, const ULONG ulInfoLevel, const PVOID pInfo, const ULONG cbInfoBuf, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosSetFileInfo(const HFILE hf, const ULONG ulInfoLevel, const PVOID pInfoBuf, const ULONG cbInfoBuf);
APIRET APIENTRY DosProtectSetFileInfo(const HFILE hf, const ULONG ulInfoLevel, const PVOID pInfoBuf, const ULONG cbInfoBuf, const FHLOCK fhFileHandleLockID);
APIRET APIENTRY DosQueryPathInfo(const PCSZ pszPathName, const ULONG ulInfoLevel, const PVOID pInfoBuf, const ULONG cbInfoBuf);
APIRET APIENTRY DosSetPathInfo(const PCSZ pszPathName, const ULONG ulInfoLevel, const PVOID pInfoBuf, const ULONG cbInfoBuf, const ULONG flOptions);
#define DSPI_WRTTHRU 0x10
APIRET APIENTRY DosShutdown(const ULONG ulReserved);
APIRET APIENTRY DosEnumAttribute(const ULONG ulRefType, const PVOID pvFile, const ULONG ulEntry, const PVOID pvBuf, const ULONG cbBuf, ULONG * pulCount, const ULONG ulInfoLevel);
APIRET APIENTRY DosProtectEnumAttribute(const ULONG ulRefType, const PVOID pvFile, const ULONG ulEntry, const PVOID pvBuf, const ULONG cbBuf, ULONG * pulCount, const ULONG ulInfoLevel, const FHLOCK fhFileHandleLockID);
typedef struct _DENA1 {
UCHAR reserved;
UCHAR cbName;
USHORT cbValue;
UCHAR szName[1];
} DENA1;
typedef DENA1 * PDENA1;
typedef FEA2 DENA2;
typedef PFEA2 PDENA2;
#define ENUMEA_LEVEL_NO_VALUE 1
#define ENUMEA_REFTYPE_FHANDLE 0
#define ENUMEA_REFTYPE_PATH 1
#define ENUMEA_REFTYPE_MAX ENUMEA_REFTYPE_PATH
#ifdef INCL_HEVDEF
  #include <hevdef.h>
#endif


#ifdef __cplusplus
   }
#endif

#endif /* __DOSFILEMGRCOMMON_H__ */
