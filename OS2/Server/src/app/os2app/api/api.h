#ifndef __OS2APP_API_H__
#define __OS2APP_API_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/dataspace.h>

/* os2app RPC includes */
#include "os2app-server.h"

long AppGetLoadError(char *uchLoadError,
                     ULONG *cbLoadError,
                     ULONG *retCode);

long AppTerminate(void);

long AppAddArea(void *addr,
                unsigned long size,
                unsigned long flags);

long AppAttachDataspace(void *addr,
                        const l4_os3_dataspace_t ds,
                        unsigned long rights);

long AppReleaseDataspace(const l4_os3_dataspace_t ds);

#ifdef __cplusplus
  }
#endif

#endif /* __OS2APP_API_H__ */
