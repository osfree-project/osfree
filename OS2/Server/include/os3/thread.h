#ifndef __OS3_THREAD_H__
#define __OS3_THREAD_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/types.h>

typedef l4_os3_cap_idx_t l4_os3_thread_t;
typedef l4_os3_thread_t  l4_os3_task_t;

typedef void (*ThreadFunc) (void *data);

#include <platform/thread.h>

BOOL ThreadEqual(l4_os3_thread_t one, l4_os3_thread_t two);

BOOL TaskEqual(l4_os3_thread_t one, l4_os3_thread_t two);

void ThreadSleep(unsigned long ms);

void ThreadExit(void);

void TaskExit(int result);

l4_os3_thread_t ThreadCreate(ThreadFunc fn, void *data, ULONG flags);

l4_os3_thread_t ThreadCreateLong(ThreadFunc fn, void *data, ULONG flags,
                                 const char *name, ULONG stacksize);

void ThreadKill(l4_os3_thread_t native);

void ThreadSuspend(l4_os3_thread_t native);

void ThreadResume(l4_os3_thread_t native);

void ThreadWait(l4_os3_thread_t native);

#ifdef __cplusplus
  }
#endif

#endif /* __OS3_THREAD_H__ */
