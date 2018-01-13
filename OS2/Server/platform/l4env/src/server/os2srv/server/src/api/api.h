/* os2srv server-side RPC API */

#ifndef __OS2SRV_API_H__
#define __OS2SRV_API_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/thread.h>
#include <os3/loader.h>

void CPAppNotify1(l4_os3_thread_t thread);

void CPAppNotify2(l4_os3_thread_t thread,
                  const os2exec_module_t *s);

APIRET CPExit(l4_os3_thread_t thread,
              ULONG action, ULONG result);

#ifdef __cplusplus
  }
#endif

#endif /* __OS2SRV_API_H__ */
