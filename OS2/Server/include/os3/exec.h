/* os2exec client-side API */

#ifndef __OS3_EXEC_H__
#define __OS3_EXEC_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/ixfmgr.h>
#include <os3/loader.h>
#include <os3/dataspace.h>

APIRET ExcClientInit(void);

APIRET ExcClientDone(void);

APIRET ExcClientOpen(PSZ              pszFileName,
                     ULONG            ulFlags,
                     PBYTE            pbLoadError,
                     PULONG           pcbLoadError,
                     PHMODULE         phmod);

APIRET ExcClientLoad(HMODULE          hmod,
                     PBYTE            pbLoadError,
                     PULONG           pcbLoadError,
                     os2exec_module_t *s);

APIRET ExcClientFree(HMODULE hmod);

APIRET ExcClientShare(HMODULE hmod);

APIRET ExcClientGetImp(HMODULE hmod,
                       ULONG *index,
                       HMODULE *imp_hmod);

APIRET ExcClientGetSect(HMODULE hmod,
                        ULONG *index,
                        l4_os3_section_t *sect);

APIRET ExcClientQueryProcAddr(HMODULE hmod,
                              ULONG ordinal,
                              PSZ pszModname,
                              void **ppfn);

APIRET ExcClientQueryModuleHandle(PSZ pszModname,
                                  HMODULE *phmod);

APIRET ExcClientQueryModuleName(HMODULE hmod,
                                ULONG cbBuf,
                                PBYTE pBuf);

APIRET ExcClientAllocSharedMem(ULONG size,
                               PSZ pszName,
                               ULONG rights,
                               void **addr,
                               ULONGLONG *area);

APIRET ExcClientMapDataspace(void *addr,
                             ULONG rights,
                             l4_os3_dataspace_t ds);

APIRET ExcClientUnmapDataspace(void *addr,
                               l4_os3_dataspace_t ds);

APIRET ExcClientGetDataspace(void **addr,
                             ULONG *size,
                             l4_os3_dataspace_t *ds);

APIRET ExcClientGetSharedMem(void *pb,
                             void **addr,
                             ULONG *size,
                             PID   *owner);
                             //l4_os3_cap_idx_t *owner);

APIRET ExcClientGetNamedSharedMem(PSZ pszName,
                                  void **addr,
                                  ULONG *size,
                                  PID   *owner);
                                  //l4_os3_cap_idx_t *owner);

APIRET ExcClientIncrementSharedMemRefcnt(void *addr);

APIRET ExcClientReleaseSharedMem(void *addr,
                                 ULONG *count);

#ifdef __cplusplus
  }
#endif

#endif
