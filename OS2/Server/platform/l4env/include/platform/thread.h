#ifndef __L4ENV_ENV_THREAD_H__
#define __L4ENV_ENV_THREAD_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* l4env includes */
#include <l4/thread/thread.h>

#define os3_thread_create l4thread_create

#define NIL_THREAD ((l4_os3_thread_t){(l4_threadid_t)L4_INVALID_ID})

#define THREAD_SYNC  L4THREAD_CREATE_SYNC
#define THREAD_ASYNC L4THREAD_CREATE_ASYNC

#ifdef __cplusplus
  }
#endif

#endif /* __L4ENV_ENV_THREAD_H__ */
