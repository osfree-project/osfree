/*  KAL functions wrappers
 */

#include <l4/log/l4log.h>

#include <gcc_os2def.h>
#include <apistub.h>
#include <dl.h>

#include <stdarg.h>
#include <dlfcn.h>

extern ULONG kalHandle;

#if 0

struct func
{
  ULONG ordinal;
  void  *addr;
} funclist[] =

{{KALORD_ALLOCMEM,                &kalAllocMem},
 {KALORD_ALLOCSHAREDMEM,          &kalAllocSharedMem},
 {KALORD_CLOSE,                   &kalClose},
 {KALORD_CREATEDIR,               &kalCreateDir},
 {KALORD_DELETE,                  &kalDelete},
 {KALORD_DELETEDIR,               &kalDeleteDir},
 {KALORD_DUPHANDLE,               &kalDupHandle},
 {KALORD_ERROR,                   &kalError},
 {KALORD_EXECPGM,                 &kalExecPgm},
 {KALORD_EXIT,                    &kalExit},
 {KALORD_FSCTL,                   &kalFSCtl},
 {KALORD_FINDCLOSE,               &kalFindClose},
 {KALORD_FINDFIRST,               &kalFindFirst},
 {KALORD_FINDNEXT,                &kalFindNext},
 {KALORD_FORCEDELETE,             &kalForceDelete},
 {KALORD_FREEMEM,                 &kalFreeMem},
 {KALORD_GETINFOBLOCKS,           &kalGetInfoBlocks},
 {KALORD_LOADMODULE,              &kalLoadModule},
 {KALORD_LOGWRITE,                &kalLogWrite},
 {KALORD_MOVE,                    &kalMove},
 {KALORD_OPENL,                   &kalOpenL},
 {KALORD_QUERYAPPTYPE,            &kalQueryAppType},
 {KALORD_QUERYCP,                 &kalQueryCp},
 {KALORD_QUERYCURRENTDIR,         &kalQueryCurrentDir},
 {KALORD_QUERYCURRENTDISK,        &kalQueryCurrentDisk},
 {KALORD_QUERYDBCSENV,            &kalQueryDBCSEnv},
 {KALORD_QUERYFHSTATE,            &kalQueryFHState},
 {KALORD_QUERYFILEINFO,           &kalQueryFileInfo},
 {KALORD_QUERYHTYPE,              &kalQueryHType},
 {KALORD_QUERYMEM,                &kalQueryMem},
 {KALORD_QUERYMODULEHANDLE,       &kalQueryModuleHandle},
 {KALORD_QUERYMODULENAME,         &kalQueryModuleName},
 {KALORD_QUERYPATHINFO,           &kalQueryPathInfo},
 {KALORD_QUERYPROCADDR,           &kalQueryProcAddr},
 {KALORD_QUERYPROCTYPE,           &kalQueryProcType},
 {KALORD_READ,                    &kalRead},
 {KALORD_RESETBUFFER,             &kalResetBuffer},
 {KALORD_SETCURRENTDIR,           &kalSetCurrentDir},
 {KALORD_SETDEFAULTDISK,          &kalSetDefaultDisk},
 {KALORD_SETFHSTATE,              &kalSetFHState},
 {KALORD_SETFILEPTRL,             &kalSetFilePtrL},
 {KALORD_SETFILESIZEL,            &kalSetFileSizeL},
 {KALORD_SETMAXFH,                &kalSetMaxFH},
 {KALORD_SETMEM,                  &kalSetMem},
 {KALORD_SETRELMAXFH,             &kalSetRelMaxFH},
 {KALORD_SLEEP,                   &kalSleep},
 {KALORD_WRITE,                   &kalWrite}};

#endif

typedef APIRET APIENTRY (*funcaddr_t)(va_list a);

APIRET CDECL DlOpen(PSZ name, PULONG handle)
{
  if (*handle = dlopen(name, 2))
    return NO_ERROR;
    
  return ERROR_FILE_NOT_FOUND;
}

APIRET CDECL DlSym(ULONG handle, PSZ sym, PPVOID addr)
{
  if (*addr = dlsym(handle, sym))
    return NO_ERROR;
    
  return ERROR_INVALID_NAME;
  
}

APIRET CDECL DlRoute(ULONG handle, PSZ name, ...)
{
  funcaddr_t addr;
  va_list args;
  APIRET rc;
  int i;

  if (!handle)
    handle = kalHandle;

  va_start(args, name);

  if (addr = (funcaddr_t)dlsym(handle, name))
    rc = addr(args);

  va_end(args);    
  return rc;
}
