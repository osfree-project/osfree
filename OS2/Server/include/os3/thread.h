#ifndef __OS2_THREAD_H__
#define __OS3_THREAD_H__

#ifdef __cplusplus
  extern "C" {
#endif

#ifdef __l4env__

/* l4env includes */
#include <l4/thread/thread.h>

#define os3_thread_create l4thread_create

#endif

#ifdef __cplusplus
  }
#endif

#endif
