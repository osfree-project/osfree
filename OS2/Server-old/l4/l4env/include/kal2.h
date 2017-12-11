#ifndef __OS3_KAL2_H__
#define __OS3_KAL2_H__

#ifdef __cplusplus
  extern "C" {
#endif

#include <l4/os3/gcc_os2def.h>

#include <stdarg.h>

void KalInit(va_list a);

APIRET CDECL
KalStartApp(va_list a);

unsigned long
KalPvtLoadModule(va_list a);

APIRET CDECL
KalRead(va_list a);

APIRET CDECL
KalWrite(va_list a);

APIRET CDECL
KalLogWrite (va_list a);

APIRET CDECL
KalFSCtl(va_list a);

VOID CDECL
KalExit(va_list a);

APIRET CDECL
KalQueryCurrentDisk(va_list a);

APIRET CDECL
KalSetCurrentDir(va_list a);

APIRET CDECL
KalSetDefaultDisk(va_list a);

APIRET CDECL
KalQueryCurrentDir(va_list a);

APIRET CDECL
KalLoadModule(va_list a);

APIRET CDECL
KalQueryProcAddr(va_list a);

APIRET CDECL
KalQueryProcType(va_list a);

APIRET CDECL
KalQueryAppType(va_list a);

APIRET CDECL
KalExecPgm(va_list a);

APIRET CDECL
KalError(va_list a);

APIRET CDECL
KalAllocMem(va_list a);

APIRET CDECL
KalFreeMem(va_list a);

APIRET CDECL
KalResetBuffer(va_list a);

APIRET CDECL
KalSetFilePtrL(va_list a);
	      
APIRET CDECL
KalClose(va_list a);

APIRET CDECL
KalSetMaxFH(va_list a);

APIRET CDECL
KalSetRelMaxFH(va_list a);

APIRET CDECL
KalFindFirst(va_list a);

APIRET CDECL
KalFindNext(va_list a);

APIRET CDECL
KalFindClose(va_list a);

APIRET CDECL
KalQueryFHState(va_list a);

APIRET CDECL
KalSetFHState(va_list a);

APIRET CDECL
KalQueryFileInfo(va_list a);

APIRET CDECL
KalQueryPathInfo(va_list a);

APIRET CDECL
KalSetFileSizeL(va_list a);

APIRET CDECL
KalAllocSharedMem(va_list a);

APIRET CDECL
KalGetSharedMem(va_list a);

APIRET CDECL
KalGetNamedSharedMem(va_list a);

APIRET CDECL
KalGiveSharedMem(va_list a);

APIRET CDECL
KalCreateDir(va_list a);

APIRET CDECL
KalDupHandle(va_list a);

APIRET CDECL
KalDelete(va_list a);

APIRET CDECL
KalForceDelete(va_list a);

APIRET CDECL
KalDeleteDir(va_list a);

APIRET CDECL
KalGetInfoBlocks(va_list a);

APIRET CDECL
KalMove(va_list a);

APIRET CDECL
KalOpenL (va_list a);

APIRET CDECL
KalQueryHType(va_list a);

APIRET CDECL
KalQueryDBCSEnv(va_list a);

APIRET CDECL
KalQueryCp(va_list a);

APIRET CDECL
KalQueryMem(va_list a);

APIRET CDECL
KalQueryModuleName(va_list a);

APIRET CDECL
KalQueryModuleHandle(va_list a);

APIRET CDECL
KalSleep(va_list a);

APIRET CDECL
KalSetMem(va_list a);

APIRET CDECL
KalOpenEventSem(va_list a);

APIRET CDECL
KalCloseEventSem(va_list a);

APIRET CDECL
KalCreateEventSem(va_list a);

APIRET CDECL
KalCreateThread(va_list a);

APIRET CDECL
KalSuspendThread(va_list a);

APIRET CDECL
KalResumeThread(va_list a);

APIRET CDECL
KalWaitThread(va_list a);

APIRET CDECL
KalKillThread(va_list a);

APIRET CDECL
KalGetTID(va_list a);

APIRET CDECL
KalGetPID(va_list a);

APIRET CDECL
KalGetL4ID(va_list a);

APIRET CDECL
KalGetTIDL4(va_list a);

APIRET CDECL
KalNewTIB(va_list a);

APIRET CDECL
KalDestroyTIB(va_list a);

APIRET CDECL
KalGetTIB(va_list a);

APIRET CDECL
KalGetPIB(va_list a);

#ifdef __cplusplus
  }
#endif

#endif
