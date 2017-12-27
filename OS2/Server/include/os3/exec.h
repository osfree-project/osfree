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
#include <os3/loader.h>
#include <os3/dataspace.h>

APIRET ExcClientInit(void);

APIRET ExcClientOpen(PSZ              pszFileName,
                     ULONG            ulFlags,
                     PBYTE            pbLoadError,
                     PULONG           pcbLoadError,
                     PHMODULE         phmod);

APIRET ExcClientLoad(HMODULE          hmod,
                     PBYTE            pbLoadError,
                     PULONG           pcbLoadError,
                     os2exec_module_t *s);

APIRET ExcClientShare(HMODULE hmod);

#ifdef __cplusplus
  }
#endif

#endif
