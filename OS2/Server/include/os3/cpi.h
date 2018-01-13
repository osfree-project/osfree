/* os2srv client-side RPC API */

#ifndef __OS3_CPI_H__
#define __OS3_CPI_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/thread.h>
#include <os3/loader.h>

void CPClientAppNotify1(void);

void CPClientAppNotify2(os2exec_module_t *s);

void CPClientExit(ULONG action, ULONG result);

#ifdef __cplusplus
  }
#endif

#endif /* __OS3_CPI_H__ */
